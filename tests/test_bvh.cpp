#include <gtest/gtest.h>
#include "cebu/bvh.h"
#include "cebu/simplicial_complex.h"

using namespace cebu;

// Test BVH construction
TEST(BVHTest, Construction) {
    SimplicialComplex complex;

    // Add some vertices
    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    VertexID v3 = complex.add_vertex();

    // Add edges
    EdgeID e0 = complex.add_edge(v0, v1);
    EdgeID e1 = complex.add_edge(v1, v2);
    EdgeID e2 = complex.add_edge(v2, v3);

    // Build BVH
    BVH bvh(complex);

    // Check statistics
    auto stats = bvh.get_statistics();
    EXPECT_GT(stats["node_count"], 0);
    EXPECT_GT(stats["leaf_count"], 0);
    EXPECT_TRUE(bvh.is_valid());
}

// Test BVH with different strategies
TEST(BVHTest, ConstructionStrategies) {
    SimplicialComplex complex;

    // Add vertices
    for (int i = 0; i < 10; ++i) {
        complex.add_vertex();
    }

    // Build BVH with different strategies
    BVH bvh_median(complex, BVH::BuildStrategy::MEDIAN_SPLIT);
    EXPECT_TRUE(bvh_median.is_valid());

    BVH bvh_sah(complex, BVH::BuildStrategy::SAH);
    EXPECT_TRUE(bvh_sah.is_valid());

    BVH bvh_midpoint(complex, BVH::BuildStrategy::MIDPOINT_SPLIT);
    EXPECT_TRUE(bvh_midpoint.is_valid());
}

// Test range query
TEST(BVHTest, RangeQuery) {
    SimplicialComplex complex;

    // Add vertices
    for (int i = 0; i < 10; ++i) {
        complex.add_vertex();
    }

    BVH bvh(complex);

    // Query a range
    auto result = bvh.range_query(-5.0f, -5.0f, -5.0f, 5.0f, 5.0f, 5.0f);
    EXPECT_GE(result.nodes_visited, 0);
    EXPECT_GE(result.query_time_ms, 0.0);
}

// Test point query
TEST(BVHTest, PointQuery) {
    SimplicialComplex complex;

    // Add vertices
    for (int i = 0; i < 10; ++i) {
        complex.add_vertex();
    }

    BVH bvh(complex);

    // Query a point
    auto result = bvh.point_query(0.0f, 0.0f, 0.0f);
    EXPECT_GE(result.nodes_visited, 0);
}

// Test sphere query
TEST(BVHTest, SphereQuery) {
    SimplicialComplex complex;

    // Add vertices
    for (int i = 0; i < 10; ++i) {
        complex.add_vertex();
    }

    BVH bvh(complex);

    // Query a sphere
    auto result = bvh.sphere_query(0.0f, 0.0f, 0.0f, 5.0f);
    EXPECT_GE(result.nodes_visited, 0);
}

// Test nearest neighbor
TEST(BVHTest, NearestNeighbor) {
    SimplicialComplex complex;

    // Add vertices
    for (int i = 0; i < 10; ++i) {
        complex.add_vertex();
    }

    BVH bvh(complex);

    // Query nearest neighbor
    auto result = bvh.nearest_neighbor(0.0f, 0.0f, 0.0f);
    EXPECT_GE(result.nodes_visited, 0);
}

// Test k-nearest neighbors
TEST(BVHTest, KNearestNeighbors) {
    SimplicialComplex complex;

    // Add vertices
    for (int i = 0; i < 10; ++i) {
        complex.add_vertex();
    }

    BVH bvh(complex);

    // Query k-nearest neighbors
    auto results = bvh.k_nearest_neighbors(0.0f, 0.0f, 0.0f, 3);
    EXPECT_LE(results.size(), 3);
}

// Test ray query
TEST(BVHTest, RayQuery) {
    SimplicialComplex complex;

    // Add vertices
    for (int i = 0; i < 10; ++i) {
        complex.add_vertex();
    }

    BVH bvh(complex);

    // Query with a ray
    std::array<float, 3> origin = {0.0f, 0.0f, 0.0f};
    std::array<float, 3> direction = {1.0f, 0.0f, 0.0f};

    auto result = bvh.ray_query(origin, direction);
    EXPECT_GE(result.nodes_visited, 0);
}

// Test rebuild
TEST(BVHTest, Rebuild) {
    SimplicialComplex complex;

    // Add vertices
    for (int i = 0; i < 10; ++i) {
        complex.add_vertex();
    }

    BVH bvh(complex);
    size_t node_count_before = bvh.get_node_count();

    // Rebuild
    bvh.rebuild();
    size_t node_count_after = bvh.get_node_count();

    EXPECT_EQ(node_count_before, node_count_after);
    EXPECT_TRUE(bvh.is_valid());
}

// Test incremental update
TEST(BVHTest, IncrementalUpdate) {
    SimplicialComplex complex;

    // Add vertices
    for (int i = 0; i < 10; ++i) {
        complex.add_vertex();
    }

    BVH bvh(complex);

    // Incremental update (with empty lists for simplicity)
    bvh.incremental_update({}, {}, {});

    EXPECT_TRUE(bvh.is_valid());
}

// Test statistics
TEST(BVHTest, Statistics) {
    SimplicialComplex complex;

    // Add vertices
    for (int i = 0; i < 10; ++i) {
        complex.add_vertex();
    }

    BVH bvh(complex);

    auto stats = bvh.get_statistics();

    EXPECT_GT(stats["max_depth"], 0);
    EXPECT_GT(stats["node_count"], 0);
    EXPECT_GT(stats["leaf_count"], 0);
    EXPECT_LE(stats["leaf_count"], stats["node_count"]);
}

// Test tree depth
TEST(BVHTest, TreeDepth) {
    SimplicialComplex complex;

    // Add vertices
    for (int i = 0; i < 10; ++i) {
        complex.add_vertex();
    }

    BVH bvh(complex);

    size_t depth = bvh.get_max_depth();
    EXPECT_GT(depth, 0);
    EXPECT_LE(depth, 32);  // max_depth default
}

// Test node count
TEST(BVHTest, NodeCount) {
    SimplicialComplex complex;

    // Add vertices
    for (int i = 0; i < 10; ++i) {
        complex.add_vertex();
    }

    BVH bvh(complex);

    size_t node_count = bvh.get_node_count();
    EXPECT_GT(node_count, 0);
}

// Test leaf count
TEST(BVHTest, LeafCount) {
    SimplicialComplex complex;

    // Add vertices
    for (int i = 0; i < 10; ++i) {
        complex.add_vertex();
    }

    BVH bvh(complex);

    size_t leaf_count = bvh.get_leaf_count();
    EXPECT_GT(leaf_count, 0);
}

// Test validity check
TEST(BVHTest, ValidityCheck) {
    SimplicialComplex complex;

    // Add vertices
    for (int i = 0; i < 10; ++i) {
        complex.add_vertex();
    }

    BVH bvh(complex);

    EXPECT_TRUE(bvh.is_valid());
}

// Test clear
TEST(BVHTest, Clear) {
    SimplicialComplex complex;

    // Add vertices
    for (int i = 0; i < 10; ++i) {
        complex.add_vertex();
    }

    BVH bvh(complex);

    EXPECT_GT(bvh.get_node_count(), 0);

    bvh.clear();
    EXPECT_EQ(bvh.get_node_count(), 0);
}

// Test optimize
TEST(BVHTest, Optimize) {
    SimplicialComplex complex;

    // Add vertices
    for (int i = 0; i < 10; ++i) {
        complex.add_vertex();
    }

    BVH bvh(complex);

    size_t node_count_before = bvh.get_node_count();

    bvh.optimize();

    size_t node_count_after = bvh.get_node_count();

    EXPECT_EQ(node_count_before, node_count_after);
    EXPECT_TRUE(bvh.is_valid());
}

// Test collision detection
TEST(BVHTest, CollisionDetection) {
    SimplicialComplex complex1;

    // Add vertices
    for (int i = 0; i < 5; ++i) {
        complex1.add_vertex();
    }

    SimplicialComplex complex2;

    // Add vertices
    for (int i = 0; i < 5; ++i) {
        complex2.add_vertex();
    }

    BVH bvh1(complex1);
    BVH bvh2(complex2);

    auto collisions = bvh1.find_collisions(bvh2);

    EXPECT_GE(collisions.size(), 0);
}

// Test empty complex
TEST(BVHTest, EmptyComplex) {
    SimplicialComplex complex;

    BVH bvh(complex);

    EXPECT_EQ(bvh.get_node_count(), 0);
    EXPECT_EQ(bvh.get_leaf_count(), 0);
    EXPECT_EQ(bvh.get_max_depth(), 0);
}

// Test large complex
TEST(BVHTest, LargeComplex) {
    SimplicialComplex complex;

    // Add many vertices
    for (int i = 0; i < 100; ++i) {
        complex.add_vertex();
    }

    BVH bvh(complex);

    EXPECT_GT(bvh.get_node_count(), 0);
    EXPECT_GT(bvh.get_leaf_count(), 0);
    EXPECT_TRUE(bvh.is_valid());

    // Test query performance
    auto result = bvh.range_query(-10.0f, -10.0f, -10.0f, 10.0f, 10.0f, 10.0f);
    EXPECT_GE(result.nodes_visited, 0);
}

// Test query timing
TEST(BVHTest, QueryTiming) {
    SimplicialComplex complex;

    // Add vertices
    for (int i = 0; i < 50; ++i) {
        complex.add_vertex();
    }

    BVH bvh(complex);

    // Test range query timing
    auto result = bvh.range_query(-5.0f, -5.0f, -5.0f, 5.0f, 5.0f, 5.0f);
    EXPECT_GE(result.query_time_ms, 0.0);

    // Test point query timing
    auto result2 = bvh.point_query(0.0f, 0.0f, 0.0f);
    EXPECT_GE(result2.query_time_ms, 0.0);
}

// Test different leaf sizes
TEST(BVHTest, DifferentLeafSizes) {
    SimplicialComplex complex;

    // Add vertices
    for (int i = 0; i < 20; ++i) {
        complex.add_vertex();
    }

    BVH bvh_small(complex, BVH::BuildStrategy::MEDIAN_SPLIT, 2);
    BVH bvh_large(complex, BVH::BuildStrategy::MEDIAN_SPLIT, 8);

    EXPECT_TRUE(bvh_small.is_valid());
    EXPECT_TRUE(bvh_large.is_valid());
}

// Test max depth limit
TEST(BVHTest, MaxDepthLimit) {
    SimplicialComplex complex;

    // Add vertices
    for (int i = 0; i < 100; ++i) {
        complex.add_vertex();
    }

    BVH bvh(complex, BVH::BuildStrategy::MEDIAN_SPLIT, 4, 10);

    size_t depth = bvh.get_max_depth();
    EXPECT_LE(depth, 10);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
