/**
 * @file phase8c_demo.cpp
 * @brief Phase 8c - 并行执行演示程序
 * 
 * 展示 Cebu 并行执行器的能力，包括：
 * - 并行 for 循环
 * - 并行归约
 * - 并行映射
 * - 性能对比
 */

#include "cebu/parallel_executor.h"
#include "cebu/simplicial_complex.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <numeric>

using namespace cebu;

// ========================================
// 辅助函数
// ========================================

template<typename Func>
double measure_time(const std::string& name, Func&& func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    double time_ms = duration.count() / 1000.0;
    
    std::cout << "[Timing] " << name << ": " << time_ms << " ms" << std::endl;
    return time_ms;
}

void print_separator(const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "  " << title << std::endl;
    std::cout << std::string(60, '=') << std::endl;
}

// ========================================
// 示例 1: 基础并行操作
// ========================================

void example1_basic_parallel_operations() {
    print_separator("Example 1: 基础并行操作");
    
    ParallelExecutor executor(4);
    
    // 并行任务执行
    std::atomic<int> counter{0};
    std::vector<std::future<void>> futures;
    
    for (int i = 0; i < 10; ++i) {
        futures.push_back(executor.enqueue([&counter, i]() {
            counter.fetch_add(1);
            std::cout << "  Task " << i << " executed" << std::endl;
        }));
    }
    
    for (auto& f : futures) {
        f.wait();
    }
    
    std::cout << "Total tasks executed: " << counter.load() << std::endl;
}

// ========================================
// 示例 2: 并行 For 循环
// ========================================

void example2_parallel_for() {
    print_separator("Example 2: 并行 For 循环");
    
    ParallelExecutor executor(4);
    
    // 生成测试数据
    std::vector<int> data(1000000);
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<int>(i);
    }
    
    // 并行处理
    auto time_parallel = measure_time("Parallel for loop", [&]() {
        executor.parallel_for(0, data.size(), [&data](size_t i) {
            data[i] = data[i] * 2 + 1;
        });
    });
    
    // 串行处理（对比）
    std::vector<int> data_serial(1000000);
    for (size_t i = 0; i < data_serial.size(); ++i) {
        data_serial[i] = static_cast<int>(i);
    }
    
    auto time_serial = measure_time("Serial for loop", [&]() {
        for (size_t i = 0; i < data_serial.size(); ++i) {
            data_serial[i] = data_serial[i] * 2 + 1;
        }
    });
    
    // 验证结果
    bool correct = (data == data_serial);
    std::cout << "Results match: " << (correct ? "Yes" : "No") << std::endl;
    std::cout << "Speedup: " << (time_serial / time_parallel) << "x" << std::endl;
}

// ========================================
// 示例 3: 并行归约
// ========================================

void example3_parallel_reduce() {
    print_separator("Example 3: 并行归约");
    
    ParallelExecutor executor(4);
    
    // 生成随机数据
    const size_t size = 1000000;
    std::vector<int> data(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 1000);
    
    for (auto& x : data) {
        x = dist(gen);
    }
    
    // 并行求和
    auto time_parallel = measure_time("Parallel reduce (sum)", [&]() {
        long long sum = executor.parallel_reduce(
            data.begin(), data.end(),
            [](long long acc, int x) { return acc + x; },
            0LL
        );
        std::cout << "  Sum (parallel): " << sum << std::endl;
    });
    
    // 串行求和
    auto time_serial = measure_time("Serial reduce (sum)", [&]() {
        long long sum = std::accumulate(data.begin(), data.end(), 0LL);
        std::cout << "  Sum (serial): " << sum << std::endl;
    });
    
    std::cout << "Speedup: " << (time_serial / time_parallel) << "x" << std::endl;
    
    // 并行最大值
    auto max_val = executor.parallel_reduce(
        data.begin(), data.end(),
        [](int acc, int x) { return std::max(acc, x); },
        std::numeric_limits<int>::min()
    );
    std::cout << "Max value: " << max_val << std::endl;
}

// ========================================
// 示例 4: 并行映射
// ========================================

void example4_parallel_map() {
    print_separator("Example 4: 并行映射");
    
    ParallelExecutor executor(4);
    
    std::vector<int> input(1000000);
    std::vector<int> output(1000000);
    
    for (size_t i = 0; i < input.size(); ++i) {
        input[i] = static_cast<int>(i);
    }
    
    auto time_parallel = measure_time("Parallel map", [&]() {
        executor.parallel_map(
            input.begin(), input.end(),
            output.begin(),
            [](int x) { return x * x; }
        );
    });
    
    // 验证一些结果
    std::cout << "First element: " << output[0] 
              << " (expected: " << (0 * 0) << ")" << std::endl;
    std::cout << "Last element: " << output.back() 
              << " (expected: " << ((input.size() - 1) * (input.size() - 1)) << ")" << std::endl;
}

// ========================================
// 示例 5: 并行单纯形复形操作
// ========================================

void example5_parallel_simplicial_complex() {
    print_separator("Example 5: 并行单纯形复形操作");
    
    ParallelExecutor executor(4);
    SimplicialComplex complex;
    
    // 并行添加顶点
    const int num_vertices = 10000;
    auto time_parallel = measure_time("Parallel add vertices", [&]() {
        std::vector<std::future<VertexID>> futures;
        for (int i = 0; i < num_vertices; ++i) {
            futures.push_back(executor.enqueue([&complex]() {
                return complex.add_vertex();
            }));
        }
        
        // 等待所有任务完成
        for (auto& f : futures) {
            f.wait();
        }
    });
    
    std::cout << "Added " << num_vertices << " vertices" << std::endl;
    std::cout << "Complex size: " << complex.simplices().size() << std::endl;
}

// ========================================
// 示例 6: 性能基准测试
// ========================================

void example6_performance_benchmark() {
    print_separator("Example 6: 性能基准测试");
    
    const std::vector<int> thread_counts = {1, 2, 4, 8};
    const size_t data_size = 10000000;
    
    std::vector<int> data(data_size);
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<int>(i % 1000);
    }
    
    std::cout << "Data size: " << data_size << " elements" << std::endl;
    std::cout << "\nThread count | Time (ms) | Speedup" << std::endl;
    std::cout << "-------------|-----------|--------" << std::endl;
    
    double base_time = 0.0;
    
    for (int num_threads : thread_counts) {
        ParallelExecutor executor(num_threads);
        
        double time = measure_time("Processing", [&]() {
            long long sum = executor.parallel_reduce(
                data.begin(), data.end(),
                [](long long acc, int x) { return acc + x; },
                0LL
            );
            (void)sum;  // 避免未使用警告
        });
        
        if (num_threads == 1) {
            base_time = time;
        }
        
        double speedup = (num_threads == 1) ? 1.0 : (base_time / time);
        double efficiency = (speedup / num_threads) * 100.0;
        
        printf("%-11d | %9.2f | %6.2fx (%.1f%%)\n", 
               num_threads, time, speedup, efficiency);
    }
}

// ========================================
// 示例 7: 全局并行执行器
// ========================================

void example7_global_executor() {
    print_separator("Example 7: 全局并行执行器");
    
    // 初始化全局执行器
    GlobalParallelExecutor::initialize(4);
    
    std::vector<int> data(100000);
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<int>(i);
    }
    
    // 使用便捷函数
    parallel_for(0, data.size(), [&data](size_t i) {
        data[i] = data[i] * 3;
    });
    
    auto sum = parallel_reduce(
        data.begin(), data.end(),
        [](int acc, int x) { return acc + x; },
        0
    );
    
    std::cout << "Sum: " << sum << std::endl;
    
    GlobalParallelExecutor::instance().wait_for_all();
    GlobalParallelExecutor::shutdown();
}

// ========================================
// 主函数
// ========================================

int main() {
    std::cout << "\n";
    std::cout << "╔═══════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║                                                           ║" << std::endl;
    std::cout << "║       Cebu Topology Library - Parallel Execution Demo    ║" << std::endl;
    std::cout << "║                    Phase 8c Implementation                ║" << std::endl;
    std::cout << "║                                                           ║" << std::cout;
    std::cout << "╚═══════════════════════════════════════════════════════════╝" << std::endl;
    
    try {
        example1_basic_parallel_operations();
        example2_parallel_for();
        example3_parallel_reduce();
        example4_parallel_map();
        example5_parallel_simplicial_complex();
        example6_performance_benchmark();
        example7_global_executor();
        
        print_separator("演示完成");
        std::cout << "\n✅ 所有示例执行成功！" << std::endl;
        std::cout << "\n📊 并行执行器已成功实现：" << std::endl;
        std::cout << "   • 并行任务调度" << std::endl;
        std::cout << "   • 并行 for 循环" << std::endl;
        std::cout << "   • 并行归约" << std::endl;
        std::cout << "   • 并行映射" << std::endl;
        std::cout << "   • 全局便捷接口" << std::endl;
        std::cout << "   • 性能优化" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "\n❌ 错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
