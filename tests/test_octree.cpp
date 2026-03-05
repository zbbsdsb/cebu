#include <gtest/gtest.h>
#include "cebu/octree.h"
#include "cebu/simplicial_complex.h"
#include "cebu/simplicial_complex_labeled.h"
#include <random>

using namespace cebu;

class OctreeTest : public ::testing::Test {
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

TEST_F(OctreeTest, DefaultConstruction) {
    Octree octree;
    EXPECT_EQ(octree.get_node_count(), 0);
    EXPECT_EQ(octree.get_simplex_count(), 0);
}

TEST_F(OctreeTest, ConstructionFromComplex) {
    Octree octree(complex);
    EXPECT_GT(octree.get_node_count(), 0);
    EXPECT_EQ(octree.get_simplex_count(), 4);
}

TEST_F(OctreeTest, ConstructionFromLabeledComplex) {
    SimplicialComplexLabeled<> labeled;
    labeled.add_vertex(0, 0.0, 0.0, 0.0);
    labeled.add_vertex(1, 1.0, 0.0, 0.0);
    labeled.add_vertex(2, 0.5, 1.0, 0.0);
    labeled.add_simplex(0, {0, 1, 2}, 0.5);

    Octree octree(labeled);
    EXPECT_GT(octree.get_node_count(), 0);
    EXPECT_EQ(octree.get_simplex_count(), 1);
}

// ============================================================================
// Point Query Tests
// ============================================================================

TEST_F(OctreeTest, QueryPointInside) {
    Octree octree(complex);
    std::array<double, 3> point = {0.5, 0.3, 0.2};
    auto results = octree.query_point(point);
    EXPECT_GT(results.size(), 0);
}

TEST_F(OctreeTest, QueryPointOutside) {
    Octree octree(complex);
    std::array<double, 3> point = {10.0, 10.0, 10.0};
    auto results = octree.query_point(point);
    EXPECT_EQ(results.size(), 0);
}

TEST_F(OctreeTest, ContainsTrue) {
    Octree octree(complex);
    std::array<double, 3> point = {0.5, 0.3, 0.2};
    EXPECT_TRUE(octree.contains(point));
}

TEST_F(OctreeTest, ContainsFalse) {
    Octree octree(complex);
    std::array<double, 3> point = {10.0, 10.0, 10.0};
    EXPECT_FALSE(octree.contains(point));
}

// ============================================================================
// Range Query Tests
// ============================================================================

TEST_F(OctreeTest, QueryRangeAll) {
    Octree octree(complex);
    auto result = octree.query_range({-1.0, -1.0, -1.0}, {2.0, 2.0, 2.0});
    EXPECT_EQ(result.simplex_ids.size(), 4);
}

TEST_F(OctreeTest, QueryRangePartial) {
    Octree octree(complex);
    auto result = octree.query_range({0.0, 0.0, 0.0}, {0.6, 0.6, 0.6});
    EXPECT_GT(result.simplex_ids.size(), 0);
    EXPECT_LE(result.simplex_ids.size(), 4);
}

TEST_F(OctreeTest, QueryRangeEmpty) {
    Octree octree(complex);
    auto result = octree.query_range({10.0, 10.0, 10.0}, {11.0, 11.0, 11.0});
    EXPECT_EQ(result.simplex_ids.size(), 0);
}

// ============================================================================
// Sphere Query Tests
// ============================================================================

TEST_F(OctreeTest, QuerySphere) {
    Octree octree(complex);
    std::array<double, 3> center = {0.5, 0.5, 0.5};
    double radius = 1.0;
    auto result = octree.query_sphere(center, radius);
    EXPECT_EQ(result.simplex_ids.size(), 4);
}

TEST_F(OctreeTest, QuerySpherePartial) {
    Octree octree(complex);
    std::array<double, 3> center = {0.0, 0.0, 0.0};
    double radius = 0.3;
    auto result = octree.query_sphere(center, radius);
    EXPECT_GE(result.simplex_ids.size(), 0);
    EXPECT_LE(result.simplex_ids.size(), 4);
}

// ============================================================================
// Nearest Neighbor Tests
// ============================================================================

TEST_F(OctreeTest, NearestNeighborsK1) {
    Octree octree(complex);
    std::array<double, 3> point = {0.5, 0.5, 0.5};
    auto results = octree.nearest_neighbors(point, 1);
    EXPECT_EQ(results.size(), 1);
}

TEST_F(OctreeTest, NearestNeighborsKAll) {
    Octree octree(complex);
    std::array<double, 3> point = {0.5, 0.5, 0.5};
    auto results = octree.nearest_neighbors(point, 10);
    EXPECT_EQ(results.size(), 4);
}

// ============================================================================
// LOD Tests
// ============================================================================

TEST_F(OctreeTest, SetLODLevel) {
    Octree octree(complex);
    octree.set_lod_level(2);
    EXPECT_EQ(octree.get_lod_level(), 2);
}

TEST_F(OctreeTest, GetLODSimplices) {
    Octree octree(complex);
    auto simplices = octree.get_lod_simplices(0);
    EXPECT_GT(simplices.size(), 0);
}

// ============================================================================
// Statistics Tests
// ============================================================================

TEST_F(OctreeTest, NodeCount) {
    Octree octree(complex);
    EXPECT_GT(octree.get_node_count(), 0);
}

TEST_F(OctreeTest, Depth) {
    Octree octree(complex);
    EXPECT_GT(octree.get_depth(), 0);
}

TEST_F(OctreeTest, LeafCount) {
    Octree octree(complex);
    EXPECT_GT(octree.get_leaf_count(), 0);
}

TEST_F(OctreeTest, SimplexCount) {
    Octree octree(complex);
    EXPECT_EQ(octree.get_simplex_count(), 4);
}

TEST_F(OctreeTest, MaxSimplicesPerNode) {
    Octree octree(complex);
    size_t max_in_node = octree.get_max_simplices_per_node();
    EXPECT_GT(max_in_node, 0);
}

// ============================================================================
// Parameter Tests
// ============================================================================

TEST_F(OctreeTest, SetMaxSimplicesPerNode) {
    Octree octree(complex);
    octree.set_max_simplices_per_node(64);
    EXPECT_EQ(octree.get_max_simplices_per_node_param(), 64);
    octree.rebuild();
    EXPECT_EQ(octree.get_max_simplices_per_node_param(), 64);
}

TEST_F(OctreeTest, SetMaxDepth) {
    Octree octree(complex);
    octree.set_max_depth(20);
    EXPECT_EQ(octree.get_max_depth_param(), 20);
    octree.rebuild();
    EXPECT_EQ(octree.get_max_depth_param(), 20);
}

// ============================================================================
// Validation Tests
// ============================================================================

TEST_F(OctreeTest, Validate) {
    Octree octree(complex);
    EXPECT_TRUE(octree.validate());
}

// ============================================================================
// Large Scale Tests
// ============================================================================

TEST(OctreeLargeScaleTest, Construction100k) {
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
    
    Octree octree(complex);
    EXPECT_EQ(octree.get_simplex_count(), 33000);
}

TEST(OctreeLargeScaleTest, QueryPerformance) {
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
    
    Octree octree(complex);
    
    // Query performance
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000; ++i) {
        std::array<double, 3> point = {25.0, 25.0, 25.0};
        octree.query_point(point);
    }
    auto end = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double>(end - start).count() * 1000.0;
    
    EXPECT_LT(elapsed, 100.0); // Should be < 100ms for 1000 queries
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
