/**
 * @file phase8d_benchmark.cpp
 * @brief Phase 8d - Performance benchmarking demonstration
 * 
 * Demonstrates the benchmarking and profiling capabilities of Cebu:
 * - Benchmark framework
 * - Performance measurement
 * - Statistical analysis
 * - Comparison between operations
 */

#include "cebu/benchmark.h"
#include "cebu/profiler.h"
#include "cebu/simplicial_complex.h"
#include "cebu/simplicial_complex_labeled.h"
#include "cebu/vertex_geometry.h"
#include "cebu/bvh.h"
#include "cebu/octree.h"
#include "cebu/parallel_executor.h"
#include <iostream>
#include <vector>
#include <random>

using namespace cebu;

// ========================================
// Benchmark configurations
// ========================================

struct TestSizes {
    size_t small{100};
    size_t medium{1000};
    size_t large{10000};
    size_t huge{100000};
};

// ========================================
// Core operation benchmarks
// ========================================

void benchmark_add_vertices() {
    std::cout << "\n=== Benchmark: Add Vertices ===\n" << std::endl;

    TestSizes sizes;

    auto benchmark = std::make_unique<Benchmark>("add_vertices_small");
    benchmark->run([&]() {
        SimplicialComplex complex;
        for (size_t i = 0; i < sizes.small; ++i) {
            complex.add_vertex();
        }
    });

    auto benchmark2 = std::make_unique<Benchmark>("add_vertices_medium");
    benchmark2->run([&]() {
        SimplicialComplex complex;
        for (size_t i = 0; i < sizes.medium; ++i) {
            complex.add_vertex();
        }
    });

    auto benchmark3 = std::make_unique<Benchmark>("add_vertices_large");
    benchmark3->run([&]() {
        SimplicialComplex complex;
        for (size_t i = 0; i < sizes.large; ++i) {
            complex.add_vertex();
        }
    });

    GlobalBenchmarkRunner::add_benchmark(std::move(benchmark));
    GlobalBenchmarkRunner::add_benchmark(std::move(benchmark2));
    GlobalBenchmarkRunner::add_benchmark(std::move(benchmark3));
}

void benchmark_add_edges() {
    std::cout << "\n=== Benchmark: Add Edges ===\n" << std::endl;

    TestSizes sizes;

    auto benchmark = std::make_unique<Benchmark>("add_edges_small");
    benchmark->setup([&sizes]() {
        SimplicialComplex complex;
        for (size_t i = 0; i < sizes.small; ++i) {
            complex.add_vertex();
        }
    });
    benchmark->run([&sizes]() {
        SimplicialComplex complex;
        for (size_t i = 0; i < sizes.small; ++i) {
            complex.add_vertex();
        }
        for (size_t i = 0; i < sizes.small - 1; ++i) {
            complex.add_edge(static_cast<VertexID>(i), 
                          static_cast<VertexID>(i + 1));
        }
    });

    auto benchmark2 = std::make_unique<Benchmark>("add_edges_medium");
    benchmark2->setup([&sizes]() {
        SimplicialComplex complex;
        for (size_t i = 0; i < sizes.medium; ++i) {
            complex.add_vertex();
        }
    });
    benchmark2->run([&sizes]() {
        SimplicialComplex complex;
        for (size_t i = 0; i < sizes.medium; ++i) {
            complex.add_vertex();
        }
        for (size_t i = 0; i < sizes.medium - 1; ++i) {
            complex.add_edge(static_cast<VertexID>(i), 
                          static_cast<VertexID>(i + 1));
        }
    });

    GlobalBenchmarkRunner::add_benchmark(std::move(benchmark));
    GlobalBenchmarkRunner::add_benchmark(std::move(benchmark2));
}

void benchmark_neighbor_query() {
    std::cout << "\n=== Benchmark: Neighbor Query ===\n" << std::endl;

    TestSizes sizes;

    auto benchmark = std::make_unique<Benchmark>("neighbor_query_small");
    benchmark->setup([&sizes]() {
        SimplicialComplex complex;
        for (size_t i = 0; i < sizes.small; ++i) {
            complex.add_vertex();
        }
        for (size_t i = 0; i < sizes.small - 1; ++i) {
            complex.add_edge(static_cast<VertexID>(i), 
                          static_cast<VertexID>(i + 1));
        }
    });
    benchmark->run([&sizes]() {
        SimplicialComplex complex;
        for (size_t i = 0; i < sizes.small; ++i) {
            complex.add_vertex();
        }
        for (size_t i = 0; i < sizes.small - 1; ++i) {
            complex.add_edge(static_cast<VertexID>(i), 
                          static_cast<VertexID>(i + 1));
        }
        // Query neighbors
        for (size_t i = 0; i < sizes.small; ++i) {
            auto neighbors = complex.neighbors(static_cast<VertexID>(i));
            (void)neighbors;
        }
    });

    GlobalBenchmarkRunner::add_benchmark(std::move(benchmark));
}

// ========================================
// Spatial indexing benchmarks
// ========================================

void benchmark_bvh_construction() {
    std::cout << "\n=== Benchmark: BVH Construction ===\n" << std::endl;

    auto benchmark = std::make_unique<Benchmark>("bvh_build_1000");
    benchmark->run([]() {
        VertexGeometry geom;
        for (int i = 0; i < 1000; ++i) {
            geom.add_vertex(i, 
                          {static_cast<double>(i), 
                           static_cast<double>(i), 
                           static_cast<double>(i)});
        }
        
        BVHTree bvh(geom);
        bvh.build();
    });

    GlobalBenchmarkRunner::add_benchmark(std::move(benchmark));
}

void benchmark_bvh_query() {
    std::cout << "\n=== Benchmark: BVH Query ===\n" << std::endl;

    auto benchmark = std::make_unique<Benchmark>("bvh_query");
    benchmark->setup([]() {
        VertexGeometry geom;
        for (int i = 0; i < 1000; ++i) {
            geom.add_vertex(i, 
                          {static_cast<double>(i), 
                           static_cast<double>(i), 
                           static_cast<double>(i)});
        }
        
        BVHTree bvh(geom);
        bvh.build();
    });
    benchmark->run([]() {
        VertexGeometry geom;
        for (int i = 0; i < 1000; ++i) {
            geom.add_vertex(i, 
                          {static_cast<double>(i), 
                           static_cast<double>(i), 
                           static_cast<double>(i)});
        }
        
        BVHTree bvh(geom);
        bvh.build();
        
        // Query points
        for (int i = 0; i < 100; ++i) {
            auto result = bvh.range_query({i - 1.0, i - 1.0, i - 1.0}, 
                                        {i + 1.0, i + 1.0, i + 1.0});
            (void)result;
        }
    });

    GlobalBenchmarkRunner::add_benchmark(std::move(benchmark));
}

// ========================================
// Parallel operation benchmarks
// ========================================

void benchmark_parallel_operations() {
    std::cout << "\n=== Benchmark: Parallel Operations ===\n" << std::endl;

    // Sequential operation
    auto benchmark = std::make_unique<Benchmark>("parallel_sequential");
    benchmark->run([]() {
        std::vector<int> data(10000);
        for (size_t i = 0; i < data.size(); ++i) {
            data[i] = static_cast<int>(i * 2);
        }
    });

    // Parallel operation with 4 threads
    auto benchmark2 = std::make_unique<Benchmark>("parallel_4_threads");
    benchmark2->run([]() {
        ParallelExecutor executor(4);
        std::vector<int> data(10000);
        executor.parallel_for(0, data.size(), [&data](size_t i) {
            data[i] = static_cast<int>(i * 2);
        });
    });

    // Parallel operation with 8 threads
    auto benchmark3 = std::make_unique<Benchmark>("parallel_8_threads");
    benchmark3->run([]() {
        ParallelExecutor executor(8);
        std::vector<int> data(10000);
        executor.parallel_for(0, data.size(), [&data](size_t i) {
            data[i] = static_cast<int>(i * 2);
        });
    });

    GlobalBenchmarkRunner::add_benchmark(std::move(benchmark));
    GlobalBenchmarkRunner::add_benchmark(std::move(benchmark2));
    GlobalBenchmarkRunner::add_benchmark(std::move(benchmark3));
}

// ========================================
// Profiling demonstration
// ========================================

void demonstrate_profiling() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "  Profiling Demonstration" << std::endl;
    std::cout << "========================================\n" << std::endl;

    SimplicialComplex complex;

    // Profile vertex addition
    {
        PROFILER_SCOPE("add_vertices");
        for (int i = 0; i < 100; ++i) {
            complex.add_vertex();
            PROFILER_INC("vertices_added");
        }
    }

    // Profile edge addition
    {
        PROFILER_SCOPE("add_edges");
        for (int i = 0; i < 99; ++i) {
            complex.add_edge(static_cast<VertexID>(i), 
                          static_cast<VertexID>(i + 1));
            PROFILER_INC("edges_added");
        }
    }

    // Profile neighbor queries
    {
        PROFILER_SCOPE("neighbor_queries");
        for (int i = 0; i < 100; ++i) {
            auto neighbors = complex.neighbors(static_cast<VertexID>(i));
            (void)neighbors;
            PROFILER_INC("neighbor_queries");
        }
    }

    // Print profiling results
    GlobalProfiler::print_stats();
}

// ========================================
// Main function
// ========================================

int main() {
    std::cout << "\n";
    std::cout << "╔═══════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║                                                           ║" << std::endl;
    std::cout << "║       Cebu Topology Library - Performance Benchmarking     ║" << std::endl;
    std::cout << "║                    Phase 8d Implementation                ║" << std::endl;
    std::cout << "║                                                           ║" << std::cout;
    std::cout << "╚═══════════════════════════════════════════════════════════╝" << std::endl;

    try {
        // Demonstrate profiling first
        demonstrate_profiling();

        // Reset profiler for benchmarks
        GlobalProfiler::reset();

        // Add benchmarks
        benchmark_add_vertices();
        benchmark_add_edges();
        benchmark_neighbor_query();
        benchmark_bvh_construction();
        benchmark_bvh_query();
        benchmark_parallel_operations();

        // Run all benchmarks
        BenchmarkConfig config;
        config.warmup_iterations = 3;
        config.min_iterations = 10;
        config.verbose = false;

        auto results = GlobalBenchmarkRunner::run_all(config);

        // Compare results
        GlobalBenchmarkRunner::instance().compare_results(results);

        // Generate report
        std::string report = GlobalBenchmarkRunner::instance().generate_report(results);
        std::cout << "\n" << report << std::endl;

        // Cleanup
        GlobalBenchmarkRunner::shutdown();

        std::cout << "\n========================================" << std::endl;
        std::cout << "  Benchmarking Complete!" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "\n✅ All benchmarks executed successfully!" << std::endl;
        std::cout << "\n📊 Key Features Demonstrated:" << std::endl;
        std::cout << "   • Benchmark framework" << std::endl;
        std::cout << "   • Performance profiling" << std::endl;
        std::cout << "   • Statistical analysis" << std::endl;
        std::cout << "   • Comparison reporting" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "\n❌ Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
