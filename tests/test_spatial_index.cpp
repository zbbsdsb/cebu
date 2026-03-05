#include <gtest/gtest.h>
#include "cebu/spatial_index.h"
#include "cebu/simplicial_complex.h"
#include "cebu/simplicial_complex_labeled.h"
#include <random>

using namespace cebu;

class SpatialIndexTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a simple tetrahedron
        complex.add_vertex(0, 0.0, 0.0, 0.0);
        complex.add_vertex(1, 1.0, 0.0, 0.0);
        complex.add_vertex(2, 0.5, 1.0, 0.0);
        complex.add_vertex(3, 0.5, 0.5, 1.0);

        complex.add_simplex(0, {0, 1, 2});
        complex.add_simplex(1, {0, 1, 3});
        complex.add_simplex(2, {0, 2, 3});
        complex.add_simplex(3, {1, 2, 3});
    }

    SimplicialComplex complex;
};

// ============================================================================
// Construction Tests
// ============================================================================

TEST_F(SpatialIndexTest, DefaultConstruction) {
    SpatialIndex index;
    EXPECT_EQ(index.get_index_type(), SpatialIndexType::AUTO);
}

TEST_F(SpatialIndexTest, ConstructionFromComplex) {
    SpatialIndex index(complex);
    EXPECT_GT(index.get_index_type(), SpatialIndexType::AUTO);
}

TEST_F(SpatialIndexTest, ConstructionFromLabeledComplex) {
    SimplicialComplexLabeled<> labeled;
    labeled.add_vertex(0, 0.0, 0.0, 0.0);
    labeled.add_vertex(1, 1.0, 0.0, 0.0);
    labeled.add_vertex(2, 0.5, 1.0, 0.0);
    labeled.add_simplex(0, {0, 1, 2}, 0.5);

    SpatialIndex index(labeled);
    EXPECT_NE(index.get_recommended_index_type(), SpatialIndexType::AUTO);
}

// ============================================================================
// Index Type Tests
// ============================================================================

TEST_F(SpatialIndexTest, SetIndexTypeBVH) {
    SpatialIndex index(complex);
    index.set_index_type(SpatialIndexType::BVH);
    EXPECT_EQ(index.get_index_type(), SpatialIndexType::BVH);
}

TEST_F(SpatialIndexTest, SetIndexTypeOctree) {
    SpatialIndex index(complex);
    index.set_index_type(SpatialIndexType::OCTREE);
    EXPECT_EQ(index.get_index_type(), SpatialIndexType::OCTREE);
}

TEST_F(SpatialIndexTest, SetIndexTypeAuto) {
    SpatialIndex index(complex);
    index.set_index_type(SpatialIndexType::AUTO);
    EXPECT_EQ(index.get_index_type(), SpatialIndexType::AUTO);
}

TEST_F(SpatialIndexTest, GetRecommendedIndexType) {
    SpatialIndex index(complex);
    SpatialIndexType recommended = index.get_recommended_index_type();
    EXPECT_NE(recommended, SpatialIndexType::AUTO);
}

// ============================================================================
// Point Query Tests
// ============================================================================

TEST_F(SpatialIndexTest, QueryPointInside) {
    SpatialIndex index(complex);
    std::array<double, 3> point = {0.5, 0.3, 0.2};
    auto results = index.query_point(point);
    EXPECT_GT(results.size(), 0);
}

TEST_F(SpatialIndexTest, QueryPointOutside) {
    SpatialIndex index(complex);
    std::array<double, 3> point = {10.0, 10.0, 10.0};
    auto results = index.query_point(point);
    EXPECT_EQ(results.size(), 0);
}

TEST_F(SpatialIndexTest, ContainsTrue) {
    SpatialIndex index(complex);
    std::array<double, 3> point = {0.5, 0.3, 0.2};
    EXPECT_TRUE(index.contains(point));
}

TEST_F(SpatialIndexTest, ContainsFalse) {
    SpatialIndex index(complex);
    std::array<double, 3> point = {10.0, 10.0, 10.0};
    EXPECT_FALSE(index.contains(point));
}

// ============================================================================
// Range Query Tests
// ============================================================================

TEST_F(SpatialIndexTest, QueryRangeAll) {
    SpatialIndex index(complex);
    auto results = index.query_range({-1.0, -1.0, -1.0}, {2.0, 2.0, 2.0});
    EXPECT_EQ(results.size(), 4);
}

TEST_F(SpatialIndexTest, QueryRangePartial) {
    SpatialIndex index(complex);
    auto results = index.query_range({0.0, 0.0, 0.0}, {0.6, 0.6, 0.6});
    EXPECT_GT(results.size(), 0);
    EXPECT_LE(results.size(), 4);
}

TEST_F(SpatialIndexTest, QueryRangeEmpty) {
    SpatialIndex index(complex);
    auto results = index.query_range({10.0, 10.0, 10.0}, {11.0, 11.0, 11.0});
    EXPECT_EQ(results.size(), 0);
}

// ============================================================================
// Sphere Query Tests
// ============================================================================

TEST_F(SpatialIndexTest, QuerySphere) {
    SpatialIndex index(complex);
    std::array<double, 3> center = {0.5, 0.5, 0.5};
    auto results = index.query_sphere(center, 1.0);
    EXPECT_EQ(results.size(), 4);
}

TEST_F(SpatialIndexTest, QuerySpherePartial) {
    SpatialIndex index(complex);
    std::array<double, 3> center = {0.0, 0.0, 0.0};
    auto results = index.query_sphere(center, 0.3);
    EXPECT_GE(results.size(), 0);
    EXPECT_LE(results.size(), 4);
}

// ============================================================================
// Nearest Neighbor Tests
// ============================================================================

TEST_F(SpatialIndexTest, NearestNeighborsK1) {
    SpatialIndex index(complex);
    std::array<double, 3> point = {0.5, 0.5, 0.5};
    auto results = index.nearest_neighbors(point, 1);
    EXPECT_EQ(results.size(), 1);
}

TEST_F(SpatialIndexTest, NearestNeighborsKAll) {
    SpatialIndex index(complex);
    std::array<double, 3> point = {0.5, 0.5, 0.5};
    auto results = index.nearest_neighbors(point, 10);
    EXPECT_EQ(results.size(), 4);
}

// ============================================================================
// Cache Tests
// ============================================================================

TEST_F(SpatialIndexTest, EnableCache) {
    SpatialIndex index(complex);
    index.enable_cache(true);
    EXPECT_TRUE(index.is_cache_enabled());
}

TEST_F(SpatialIndexTest, DisableCache) {
    SpatialIndex index(complex);
    index.enable_cache(false);
    EXPECT_FALSE(index.is_cache_enabled());
}

TEST_F(SpatialIndexTest, ClearCache) {
    SpatialIndex index(complex);
    index.enable_cache(true);
    std::array<double, 3> point = {0.5, 0.3, 0.2};
    index.query_point(point);
    index.query_point(point);
    EXPECT_GT(index.get_cache_size(), 0);
    
    index.clear_cache();
    EXPECT_EQ(index.get_cache_size(), 0);
}

TEST_F(SpatialIndexTest, SetCacheSize) {
    SpatialIndex index(complex);
    index.set_cache_size(500);
    EXPECT_EQ(index.get_cache_size(), 0); // Initially empty
}

TEST_F(SpatialIndexTest, CacheHitRate) {
    SpatialIndex index(complex);
    index.enable_cache(true);
    
    // First query (cache miss)
    std::array<double, 3> point = {0.5, 0.3, 0.2};
    index.query_point(point);
    
    // Second query (cache hit)
    index.query_point(point);
    
    double hit_rate = index.get_cache_hit_rate();
    EXPECT_GT(hit_rate, 0.0);
}

// ============================================================================
// Profiling Tests
// ============================================================================

TEST_F(SpatialIndexTest, EnableProfiling) {
    SpatialIndex index(complex);
    index.enable_profiling(true);
    EXPECT_TRUE(index.is_profiling_enabled());
}

TEST_F(SpatialIndexTest, DisableProfiling) {
    SpatialIndex index(complex);
    index.enable_profiling(false);
    EXPECT_FALSE(index.is_profiling_enabled());
}

TEST_F(SpatialIndexTest, GetMetrics) {
    SpatialIndex index(complex);
    index.enable_profiling(true);
    
    std::array<double, 3> point = {0.5, 0.3, 0.2};
    index.query_point(point);
    
    SpatialIndexMetrics metrics = index.get_metrics();
    EXPECT_GT(metrics.total_queries, 0);
    EXPECT_GT(metrics.avg_query_time_ms, 0.0);
}

TEST_F(SpatialIndexTest, ResetMetrics) {
    SpatialIndex index(complex);
    index.enable_profiling(true);
    
    std::array<double, 3> point = {0.5, 0.3, 0.2};
    index.query_point(point);
    
    index.reset_metrics();
    
    SpatialIndexMetrics metrics = index.get_metrics();
    EXPECT_EQ(metrics.total_queries, 0);
}

// ============================================================================
// Data Analysis Tests
// ============================================================================

TEST_F(SpatialIndexTest, AnalyzeDistribution) {
    SpatialIndex index(complex);
    DataDistribution dist = index.analyze_distribution();
    EXPECT_GT(dist.simplex_count, 0);
}

TEST_F(SpatialIndexTest, PrintAnalysis) {
    SpatialIndex index(complex);
    // Should not crash
    index.print_analysis();
}

// ============================================================================
// Adaptive Optimization Tests
// ============================================================================

TEST_F(SpatialIndexTest, EnableAdaptiveOptimization) {
    SpatialIndex index(complex);
    index.enable_adaptive_optimization(true);
    EXPECT_TRUE(index.is_adaptive_optimization_enabled());
}

TEST_F(SpatialIndexTest, DisableAdaptiveOptimization) {
    SpatialIndex index(complex);
    index.enable_adaptive_optimization(false);
    EXPECT_FALSE(index.is_adaptive_optimization_enabled());
}

TEST_F(SpatialIndexTest, TriggerOptimization) {
    SpatialIndex index(complex);
    index.enable_adaptive_optimization(true);
    // Should not crash
    index.trigger_optimization();
}

// ============================================================================
// Hotspot Tests
// ============================================================================

TEST_F(SpatialIndexTest, IdentifyHotspots) {
    SpatialIndex index(complex);
    index.enable_cache(true);
    
    // Query same point multiple times
    std::array<double, 3> point = {0.5, 0.3, 0.2};
    for (int i = 0; i < 10; ++i) {
        index.query_point(point);
    }
    
    // Should not crash
    index.identify_hotspots();
}

TEST_F(SpatialIndexTest, GetHotspots) {
    SpatialIndex index(complex);
    index.enable_cache(true);
    
    // Query same point multiple times
    std::array<double, 3> point = {0.5, 0.3, 0.2};
    for (int i = 0; i < 10; ++i) {
        index.query_point(point);
    }
    
    auto hotspots = index.get_hotspots(10);
    EXPECT_GE(hotspots.size(), 0);
}

// ============================================================================
// Validation Tests
// ============================================================================

TEST_F(SpatialIndexTest, Validate) {
    SpatialIndex index(complex);
    EXPECT_TRUE(index.validate());
}

// ============================================================================
// Statistics Tests
// ============================================================================

TEST_F(SpatialIndexTest, PrintStatistics) {
    SpatialIndex index(complex);
    // Should not crash
    index.print_statistics();
}

// ============================================================================
// Rebuild Tests
// ============================================================================

TEST_F(SpatialIndexTest, Rebuild) {
    SpatialIndex index(complex);
    index.rebuild();
    EXPECT_TRUE(index.validate());
}

TEST_F(SpatialIndexTest, RebuildWithType) {
    SpatialIndex index(complex);
    index.rebuild(SpatialIndexType::BVH);
    EXPECT_TRUE(index.validate());
}

// ============================================================================
// Large Scale Tests
// ============================================================================

TEST(SpatialIndexLargeScaleTest, Construction100k) {
    SimplicialComplex complex;
    
    // Create 100k random vertices
    std::mt19937 gen(42);
    std::uniform_real_distribution<> dis(-10.0, 10.0);
    
    for (size_t i = 0; i < 100000; ++i) {
        complex.add_vertex(i, dis(gen), dis(gen), dis(gen));
    }
    
    // Create triangles
    for (size_t i = 0; i < 33000; ++i) {
        complex.add_simplex(i, {i*3, i*3+1, i*3+2});
    }
    
    SpatialIndex index(complex);
    EXPECT_NE(index.get_recommended_index_type(), SpatialIndexType::AUTO);
}

TEST(SpatialIndexLargeScaleTest, QueryPerformance) {
    SimplicialComplex complex;
    
    // Create grid of vertices
    size_t grid_size = 50;
    for (size_t x = 0; x < grid_size; ++x) {
        for (size_t y = 0; y < grid_size; ++y) {
            for (size_t z = 0; z < grid_size; ++z) {
                size_t id = x * grid_size * grid_size + y * grid_size + z;
                complex.add_vertex(id, static_cast<double>(x),
                                   static_cast<double>(y),
                                   static_cast<double>(z));
            }
        }
    }
    
    // Create tetrahedra
    size_t simplex_count = 0;
    for (size_t x = 0; x < grid_size - 1; ++x) {
        for (size_t y = 0; y < grid_size - 1; ++y) {
            for (size_t z = 0; z < grid_size - 1; ++z) {
                complex.add_simplex(simplex_count++, {
                    x * grid_size * grid_size + y * grid_size + z,
                    (x + 1) * grid_size * grid_size + y * grid_size + z,
                    x * grid_size * grid_size + (y + 1) * grid_size + z,
                    x * grid_size * grid_size + y * grid_size + (z + 1)
                });
            }
        }
    }
    
    SpatialIndex index(complex);
    index.enable_profiling(true);
    
    // Query performance
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000; ++i) {
        std::array<double, 3> point = {25.0, 25.0, 25.0};
        index.query_point(point);
    }
    auto end = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double>(end - start).count() * 1000.0;
    
    EXPECT_LT(elapsed, 100.0); // Should be < 100ms for 1000 queries
    
    SpatialIndexMetrics metrics = index.get_metrics();
    EXPECT_EQ(metrics.total_queries, 1000);
}

TEST(SpatialIndexLargeScaleTest, CachePerformance) {
    SimplicialComplex complex;
    
    // Create 1000 simplices
    for (size_t i = 0; i < 3000; ++i) {
        complex.add_vertex(i, static_cast<double>(i % 100),
                           static_cast<double>((i / 100) % 100),
                           static_cast<double>(i / 10000));
    }
    
    for (size_t i = 0; i < 1000; ++i) {
        complex.add_simplex(i, {i*3, i*3+1, i*3+2});
    }
    
    SpatialIndex index(complex);
    index.enable_cache(true);
    index.enable_profiling(true);
    
    // First pass (cache misses)
    for (size_t i = 0; i < 100; ++i) {
        std::array<double, 3> point = {static_cast<double>(i % 100),
                                       static_cast<double>((i / 10) % 100),
                                       0.0};
        index.query_point(point);
    }
    
    double hit_rate_no_cache = index.get_cache_hit_rate();
    
    // Second pass (cache hits)
    for (size_t i = 0; i < 100; ++i) {
        std::array<double, 3> point = {static_cast<double>(i % 100),
                                       static_cast<double>((i / 10) % 100),
                                       0.0};
        index.query_point(point);
    }
    
    double hit_rate_with_cache = index.get_cache_hit_rate();
    
    // Cache should improve hit rate
    EXPECT_GT(hit_rate_with_cache, hit_rate_no_cache);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
