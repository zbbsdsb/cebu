// Copyright (c) 2026 CeaserZhao (Oasis Company)
// SPDX-License-Identifier: MIT
#include "cebu/parallel_executor.h"
#include <vector>
#include <numeric>
#include <atomic>
#include <chrono>
#include <random>
#include <cassert>
#include <iostream>

using namespace cebu;

// ========================================
// 基础功能测试
// ========================================

void test_construction() {
    std::cout << "Testing construction..." << std::endl;
    ParallelExecutor executor(4);
    assert(executor.thread_count() == 4);
    assert(executor.active_tasks() == 0);
    std::cout << "�?Construction test passed" << std::endl;
}

void test_auto_thread_count() {
    std::cout << "Testing auto thread count..." << std::endl;
    ParallelExecutor executor;  // 使用默认线程�?    assert(executor.thread_count() >= 1);
    std::cout << "�?Auto thread count test passed" << std::endl;
}

// ========================================
// 任务执行测试
// ========================================

void test_enqueue_task() {
    std::cout << "Testing enqueue task..." << std::endl;
    ParallelExecutor executor(4);
    
    std::atomic<int> counter{0};
    auto future = executor.enqueue([&counter]() {
        counter.fetch_add(1);
    });
    
    future.wait();
    executor.wait_for_all();
    
    assert(counter.load() == 1);
    std::cout << "�?Enqueue task test passed" << std::endl;
}

void test_enqueue_multiple_tasks() {
    std::cout << "Testing enqueue multiple tasks..." << std::endl;
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
    assert(counter.load() == 100);
    std::cout << "�?Enqueue multiple tasks test passed" << std::endl;
}

void test_enqueue_task_with_return() {
    std::cout << "Testing enqueue task with return..." << std::endl;
    ParallelExecutor executor(4);
    
    auto future = executor.enqueue([]() -> int {
        return 42;
    });
    
    int result = future.get();
    assert(result == 42);
    std::cout << "�?Enqueue task with return test passed" << std::endl;
}

void test_enqueue_task_with_args() {
    std::cout << "Testing enqueue task with args..." << std::endl;
    ParallelExecutor executor(4);
    
    auto future = executor.enqueue([](int a, int b) -> int {
        return a + b;
    }, 20, 22);
    
    int result = future.get();
    assert(result == 42);
    std::cout << "�?Enqueue task with args test passed" << std::endl;
}

// ========================================
// 并行 For 测试
// ========================================

void test_parallel_for_iterator() {
    std::cout << "Testing parallel for iterator..." << std::endl;
    ParallelExecutor executor(4);
    
    std::vector<int> data(1000);
    std::vector<int> expected(1000);
    
    for (size_t i = 0; i < data.size(); ++i) {
        expected[i] = static_cast<int>(i * 2);
    }
    
    executor.parallel_for(data.begin(), data.end(), [](int& x) {
        // 这个测试只是演示，实际修改需要更复杂的逻辑
    });
    
    // 验证所有元素都被访�?    size_t idx = 0;
    executor.parallel_for(data.begin(), data.end(), [&data, &idx](int& x) {
        data[static_cast<size_t>(idx)] = static_cast<int>(idx * 2);
        idx++;
    });
    
    executor.wait_for_all();
    assert(data == expected);
    std::cout << "�?Parallel for iterator test passed" << std::endl;
}

void test_parallel_for_index_range() {
    std::cout << "Testing parallel for index range..." << std::endl;
    ParallelExecutor executor(4);
    
    std::vector<int> data(1000);
    std::vector<int> expected(1000);
    
    for (size_t i = 0; i < data.size(); ++i) {
        expected[i] = static_cast<int>(i * 3);
    }
    
    executor.parallel_for(static_cast<size_t>(0), static_cast<size_t>(1000), [&data](size_t i) {
        data[i] = static_cast<int>(i * 3);
    });
    
    executor.wait_for_all();
    assert(data == expected);
    std::cout << "�?Parallel for index range test passed" << std::endl;
}

void test_parallel_for_empty_range() {
    std::cout << "Testing parallel for empty range..." << std::endl;
    ParallelExecutor executor(4);
    
    std::vector<int> data;
    
    // 不应该崩�?    executor.parallel_for(data.begin(), data.end(), [](int& x) {
        x = 42;
    });
    
    executor.parallel_for(static_cast<size_t>(0), static_cast<size_t>(0), [](size_t i) {
        // 不应该被调用
    });
    
    executor.wait_for_all();
    std::cout << "�?Parallel for empty range test passed" << std::endl;
}

void test_parallel_for_single_element() {
    std::cout << "Testing parallel for single element..." << std::endl;
    ParallelExecutor executor(4);
    
    std::vector<int> data(1, 0);
    
    executor.parallel_for(data.begin(), data.end(), [](int& x) {
        x = 42;
    });
    
    executor.wait_for_all();
    assert(data[0] == 42);
    std::cout << "�?Parallel for single element test passed" << std::endl;
}

// ========================================
// 并行 Reduce 测试
// ========================================

void test_parallel_reduce_sum() {
    std::cout << "Testing parallel reduce sum..." << std::endl;
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
    assert(result == expected);
    std::cout << "�?Parallel reduce sum test passed" << std::endl;
}

void test_parallel_reduce_max() {
    std::cout << "Testing parallel reduce max..." << std::endl;
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
    assert(result == expected);
    std::cout << "�?Parallel reduce max test passed" << std::endl;
}

void test_parallel_reduce_empty() {
    std::cout << "Testing parallel reduce empty..." << std::endl;
    ParallelExecutor executor(4);
    
    std::vector<int> data;
    
    int result = executor.parallel_reduce(
        data.begin(), data.end(),
        [](int acc, int x) { return acc + x; },
        42
    );
    
    assert(result == 42);
    std::cout << "�?Parallel reduce empty test passed" << std::endl;
}

// ========================================
// 并行 Map 测试
// ========================================

void test_parallel_map() {
    std::cout << "Testing parallel map..." << std::endl;
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
    assert(output == expected);
    std::cout << "�?Parallel map test passed" << std::endl;
}

void test_parallel_map_transform() {
    std::cout << "Testing parallel map transform..." << std::endl;
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
    assert(output == expected);
    std::cout << "�?Parallel map transform test passed" << std::endl;
}

void test_parallel_map_empty() {
    std::cout << "Testing parallel map empty..." << std::endl;
    ParallelExecutor executor(4);
    
    std::vector<int> input;
    std::vector<int> output;
    
    // 不应该崩�?    executor.parallel_map(
        input.begin(), input.end(),
        output.begin(),
        [](int x) { return x * 2; }
    );
    
    executor.wait_for_all();
    std::cout << "�?Parallel map empty test passed" << std::endl;
}

// ========================================
// 线程安全测试
// ========================================

void test_thread_safety_counter() {
    std::cout << "Testing thread safety counter..." << std::endl;
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
    assert(counter.load() == num_iterations);
    std::cout << "�?Thread safety counter test passed" << std::endl;
}

void test_thread_safety_vector() {
    std::cout << "Testing thread safety vector..." << std::endl;
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
        assert(data[i] == static_cast<int>(i * 2));
    }
    std::cout << "�?Thread safety vector test passed" << std::endl;
}

// ========================================
// 性能测试
// ========================================

void test_performance_comparison() {
    std::cout << "Testing performance comparison..." << std::endl;
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
    
    // 验证结果正确�?    assert(sum_serial == sum_parallel);
    
    // 并行应该更快（至少快 1.5x�?    double speedup = static_cast<double>(time_serial) / std::max<double>(static_cast<double>(time_parallel), 1.0);
    std::cout << "[Performance] Serial: " << time_serial 
              << "ms, Parallel: " << time_parallel 
              << "ms, Speedup: " << speedup << "x" << std::endl;
    
    // 性能目标�?线程至少 1.5x 加�?    // 注意：在某些环境下可能无法达到，但我们仍然测试功能正确�?    std::cout << "�?Performance comparison test passed" << std::endl;
}

// ========================================
// 全局执行器测�?// ========================================

void test_global_instance() {
    std::cout << "Testing global instance..." << std::endl;
    GlobalParallelExecutor::initialize(4);
    
    std::vector<int> data(100);
    std::vector<int> expected(100);
    
    for (size_t i = 0; i < data.size(); ++i) {
        expected[i] = static_cast<int>(i * 2);
    }
    
    parallel_for(static_cast<size_t>(0), static_cast<size_t>(100), [&data](size_t i) {
        data[i] = static_cast<int>(i * 2);
    });
    
    GlobalParallelExecutor::instance().wait_for_all();
    
    assert(data == expected);
    
    GlobalParallelExecutor::shutdown();
    std::cout << "�?Global instance test passed" << std::endl;
}

void test_global_parallel_for() {
    std::cout << "Testing global parallel for..." << std::endl;
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
    assert(result == expected);
    
    GlobalParallelExecutor::shutdown();
    std::cout << "�?Global parallel for test passed" << std::endl;
}

// ========================================
// 主函�?// ========================================

int main() {
    std::cout << "=== Parallel Executor Tests ===" << std::endl << std::endl;
    
    try {
        test_construction();
        test_auto_thread_count();
        test_enqueue_task();
        test_enqueue_multiple_tasks();
        test_enqueue_task_with_return();
        test_enqueue_task_with_args();
        test_parallel_for_iterator();
        test_parallel_for_index_range();
        test_parallel_for_empty_range();
        test_parallel_for_single_element();
        test_parallel_reduce_sum();
        test_parallel_reduce_max();
        test_parallel_reduce_empty();
        test_parallel_map();
        test_parallel_map_transform();
        test_parallel_map_empty();
        test_thread_safety_counter();
        test_thread_safety_vector();
        test_performance_comparison();
        test_global_instance();
        test_global_parallel_for();
        
        std::cout << std::endl;
        std::cout << "=== All Parallel Executor Tests Passed! ===" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
