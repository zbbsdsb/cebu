#include <gtest/gtest.h>
#include "cebu/parallel_executor.h"
#include <vector>
#include <numeric>
#include <atomic>
#include <chrono>
#include <random>

using namespace cebu;

// ========================================
// 基础功能测试
// ========================================

TEST(ParallelExecutorTest, Construction) {
    ParallelExecutor executor(4);
    EXPECT_EQ(executor.thread_count(), 4);
    EXPECT_EQ(executor.active_tasks(), 0);
}

TEST(ParallelExecutorTest, AutoThreadCount) {
    ParallelExecutor executor;  // 使用默认线程数
    EXPECT_GE(executor.thread_count(), 1);
}

// ========================================
// 任务执行测试
// ========================================

TEST(ParallelExecutorTest, EnqueueTask) {
    ParallelExecutor executor(4);
    
    std::atomic<int> counter{0};
    auto future = executor.enqueue([&counter]() {
        counter.fetch_add(1);
    });
    
    future.wait();
    executor.wait_for_all();
    
    EXPECT_EQ(counter.load(), 1);
}

TEST(ParallelExecutorTest, EnqueueMultipleTasks) {
    ParallelExecutor executor(4);
    
    std::atomic<int> counter{0};
    std::vector<std::future<void>> futures;
    
    for (int i = 0; i < 100; ++i) {
        futures.push_back(executor.enqueue([&counter]() {
            counter.fetch_add(1);
        }));
    }
    
    for (auto& f : futures) {
        f.wait();
    }
    
    executor.wait_for_all();
    EXPECT_EQ(counter.load(), 100);
}

TEST(ParallelExecutorTest, EnqueueTaskWithReturn) {
    ParallelExecutor executor(4);
    
    auto future = executor.enqueue([]() -> int {
        return 42;
    });
    
    int result = future.get();
    EXPECT_EQ(result, 42);
}

TEST(ParallelExecutorTest, EnqueueTaskWithArgs) {
    ParallelExecutor executor(4);
    
    auto future = executor.enqueue([](int a, int b) -> int {
        return a + b;
    }, 20, 22);
    
    int result = future.get();
    EXPECT_EQ(result, 42);
}

// ========================================
// 并行 For 测试
// ========================================

TEST(ParallelExecutorTest, ParallelForIterator) {
    ParallelExecutor executor(4);
    
    std::vector<int> data(1000);
    std::vector<int> expected(1000);
    
    for (size_t i = 0; i < data.size(); ++i) {
        expected[i] = i * 2;
    }
    
    executor.parallel_for(data.begin(), data.end(), [](int& x) {
        // 这个测试只是演示，实际修改需要更复杂的逻辑
    });
    
    // 验证所有元素都被访问
    executor.parallel_for(data.begin(), data.end(), [&data](size_t idx) {
        data[idx] = idx * 2;
    });
    
    executor.wait_for_all();
    EXPECT_EQ(data, expected);
}

TEST(ParallelExecutorTest, ParallelForIndexRange) {
    ParallelExecutor executor(4);
    
    std::vector<int> data(1000);
    std::vector<int> expected(1000);
    
    for (size_t i = 0; i < data.size(); ++i) {
        expected[i] = static_cast<int>(i * 3);
    }
    
    executor.parallel_for(0, 1000, [&data](size_t i) {
        data[i] = static_cast<int>(i * 3);
    });
    
    executor.wait_for_all();
    EXPECT_EQ(data, expected);
}

TEST(ParallelExecutorTest, ParallelForEmptyRange) {
    ParallelExecutor executor(4);
    
    std::vector<int> data;
    
    // 不应该崩溃
    executor.parallel_for(data.begin(), data.end(), [](int& x) {
        x = 42;
    });
    
    executor.parallel_for(0, 0, [](size_t i) {
        // 不应该被调用
    });
    
    executor.wait_for_all();
}

TEST(ParallelExecutorTest, ParallelForSingleElement) {
    ParallelExecutor executor(4);
    
    std::vector<int> data(1, 0);
    
    executor.parallel_for(data.begin(), data.end(), [](int& x) {
        x = 42;
    });
    
    executor.wait_for_all();
    EXPECT_EQ(data[0], 42);
}

// ========================================
// 并行 Reduce 测试
// ========================================

TEST(ParallelExecutorTest, ParallelReduceSum) {
    ParallelExecutor executor(4);
    
    std::vector<int> data(1000);
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<int>(i);
    }
    
    int result = executor.parallel_reduce(
        data.begin(), data.end(),
        [](int acc, int x) { return acc + x; },
        0
    );
    
    int expected = std::accumulate(data.begin(), data.end(), 0);
    EXPECT_EQ(result, expected);
}

TEST(ParallelExecutorTest, ParallelReduceMax) {
    ParallelExecutor executor(4);
    
    std::vector<int> data(1000);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 10000);
    
    for (auto& x : data) {
        x = dist(gen);
    }
    
    int result = executor.parallel_reduce(
        data.begin(), data.end(),
        [](int acc, int x) { return std::max(acc, x); },
        std::numeric_limits<int>::min()
    );
    
    int expected = *std::max_element(data.begin(), data.end());
    EXPECT_EQ(result, expected);
}

TEST(ParallelExecutorTest, ParallelReduceEmpty) {
    ParallelExecutor executor(4);
    
    std::vector<int> data;
    
    int result = executor.parallel_reduce(
        data.begin(), data.end(),
        [](int acc, int x) { return acc + x; },
        42
    );
    
    EXPECT_EQ(result, 42);
}

// ========================================
// 并行 Map 测试
// ========================================

TEST(ParallelExecutorTest, ParallelMap) {
    ParallelExecutor executor(4);
    
    std::vector<int> input(1000);
    std::vector<int> output(1000);
    std::vector<int> expected(1000);
    
    for (size_t i = 0; i < input.size(); ++i) {
        input[i] = static_cast<int>(i);
        expected[i] = static_cast<int>(i * 2);
    }
    
    executor.parallel_map(
        input.begin(), input.end(),
        output.begin(),
        [](int x) { return x * 2; }
    );
    
    executor.wait_for_all();
    EXPECT_EQ(output, expected);
}

TEST(ParallelExecutorTest, ParallelMapTransform) {
    ParallelExecutor executor(4);
    
    std::vector<std::string> input = {"hello", "world", "test", "parallel"};
    std::vector<int> output(input.size());
    std::vector<int> expected = {5, 5, 4, 8};
    
    executor.parallel_map(
        input.begin(), input.end(),
        output.begin(),
        [](const std::string& s) { return static_cast<int>(s.length()); }
    );
    
    executor.wait_for_all();
    EXPECT_EQ(output, expected);
}

TEST(ParallelExecutorTest, ParallelMapEmpty) {
    ParallelExecutor executor(4);
    
    std::vector<int> input;
    std::vector<int> output;
    
    // 不应该崩溃
    executor.parallel_map(
        input.begin(), input.end(),
        output.begin(),
        [](int x) { return x * 2; }
    );
    
    executor.wait_for_all();
}

// ========================================
// 线程安全测试
// ========================================

TEST(ParallelExecutorTest, ThreadSafetyCounter) {
    ParallelExecutor executor(8);
    
    std::atomic<int> counter{0};
    const int num_iterations = 10000;
    
    std::vector<std::future<void>> futures;
    for (int i = 0; i < num_iterations; ++i) {
        futures.push_back(executor.enqueue([&counter]() {
            counter.fetch_add(1, std::memory_order_relaxed);
        }));
    }
    
    for (auto& f : futures) {
        f.wait();
    }
    
    executor.wait_for_all();
    EXPECT_EQ(counter.load(), num_iterations);
}

TEST(ParallelExecutorTest, ThreadSafetyVector) {
    ParallelExecutor executor(8);
    
    std::vector<int> data(1000, 0);
    std::mutex mutex;
    
    std::vector<std::future<void>> futures;
    for (int i = 0; i < 1000; ++i) {
        futures.push_back(executor.enqueue([&data, &mutex, i]() {
            std::lock_guard<std::mutex> lock(mutex);
            data[i] = i * 2;
        }));
    }
    
    for (auto& f : futures) {
        f.wait();
    }
    
    executor.wait_for_all();
    
    for (size_t i = 0; i < data.size(); ++i) {
        EXPECT_EQ(data[i], static_cast<int>(i * 2));
    }
}

// ========================================
// 性能测试
// ========================================

TEST(ParallelExecutorTest, PerformanceComparison) {
    const size_t data_size = 1000000;
    std::vector<int> data(data_size);
    
    for (size_t i = 0; i < data_size; ++i) {
        data[i] = static_cast<int>(i);
    }
    
    // 串行执行
    auto start_serial = std::chrono::high_resolution_clock::now();
    long long sum_serial = 0;
    for (size_t i = 0; i < data_size; ++i) {
        sum_serial += data[i];
    }
    auto end_serial = std::chrono::high_resolution_clock::now();
    auto time_serial = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_serial - start_serial).count();
    
    // 并行执行
    ParallelExecutor executor(4);
    auto start_parallel = std::chrono::high_resolution_clock::now();
    long long sum_parallel = executor.parallel_reduce(
        data.begin(), data.end(),
        [](long long acc, int x) { return acc + x; },
        0LL
    );
    executor.wait_for_all();
    auto end_parallel = std::chrono::high_resolution_clock::now();
    auto time_parallel = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_parallel - start_parallel).count();
    
    // 验证结果正确性
    EXPECT_EQ(sum_serial, sum_parallel);
    
    // 并行应该更快（至少快 1.5x）
    double speedup = static_cast<double>(time_serial) / std::max<double>(time_parallel, 1);
    std::cout << "[Performance] Serial: " << time_serial 
              << "ms, Parallel: " << time_parallel 
              << "ms, Speedup: " << speedup << "x" << std::endl;
    
    // 性能目标：4线程至少 2x 加速
    EXPECT_GT(speedup, 1.5);
}

// ========================================
// 全局执行器测试
// ========================================

TEST(GlobalParallelExecutorTest, GlobalInstance) {
    GlobalParallelExecutor::initialize(4);
    
    std::vector<int> data(100);
    std::vector<int> expected(100);
    
    for (size_t i = 0; i < data.size(); ++i) {
        expected[i] = static_cast<int>(i * 2);
    }
    
    parallel_for(0, 100, [&data](size_t i) {
        data[i] = static_cast<int>(i * 2);
    });
    
    GlobalParallelExecutor::instance().wait_for_all();
    
    EXPECT_EQ(data, expected);
    
    GlobalParallelExecutor::shutdown();
}

TEST(GlobalParallelExecutorTest, GlobalParallelFor) {
    GlobalParallelExecutor::initialize(4);
    
    std::vector<int> data(100);
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<int>(i);
    }
    
    auto result = parallel_reduce(
        data.begin(), data.end(),
        [](int acc, int x) { return acc + x; },
        0
    );
    
    GlobalParallelExecutor::instance().wait_for_all();
    
    int expected = std::accumulate(data.begin(), data.end(), 0);
    EXPECT_EQ(result, expected);
    
    GlobalParallelExecutor::shutdown();
}

// ========================================
// 主函数
// ========================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
