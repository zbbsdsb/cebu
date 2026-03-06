#include "cebu/bvh.h"
#include "cebu/simplicial_complex.h"
#include <iostream>
#include <cassert>
#include <random>
#include <chrono>

using namespace cebu;

void test_bvh_construction() {
    std::cout << "Testing BVH construction..." << std::endl;

    // Create a simplicial complex
    SimplicialComplex complex;

    // Add vertices with geometry
    VertexGeometry geometry;
    for (int i = 0; i < 100; ++i) {
        VertexID v = complex.add_vertex();
        // Random positions in [0, 10] cube
        float x = static_cast<float>(rand()) / RAND_MAX * 10.0f;
        float y = static_cast<float>(rand()) / RAND_MAX * 10.0f;
        float z = static_cast<float>(rand()) / RAND_MAX * 10.0f;
        geometry.set_position(v, x, y, z);
    }

    // Add some triangles
    std::vector<SimplexID> triangle_ids;
    for (int i = 0; i < 50; ++i) {
        VertexID v1 = i * 2 % 100;
        VertexID v2 = (i * 2 + 1) % 100;
        VertexID v3 = (i * 2 + 2) % 100;
        SimplexID tid = complex.add_triangle(v1, v2, v3);
        triangle_ids.push_back(tid);
    }

    // Build BVH
    BVHTree bvh(BVHBuildStrategy::SAH, 8, 20);
    bvh.build(triangle_ids, geometry, [&complex](SimplexID id) {
        return complex.get_simplex(id).vertices();
    });

    // Check BVH is built
    assert(bvh.is_built());
    assert(bvh.node_count() > 0);
    assert(bvh.simplex_count() == triangle_ids.size());

    std::cout << "  BVH construction: PASSED" << std::endl;
}

void test_bvh_range_query() {
    std::cout << "Testing BVH range query..." << std::endl;

    SimplicialComplex complex;
    VertexGeometry geometry;

    // Create a grid of vertices
    for (int x = 0; x < 10; ++x) {
        for (int y = 0; y < 10; ++y) {
            for (int z = 0; z < 10; ++z) {
                VertexID v = complex.add_vertex();
                geometry.set_position(v, static_cast<float>(x),
                                        static_cast<float>(y),
                                        static_cast<float>(z));
            }
        }
    }

    // Add triangles
    std::vector<SimplexID> triangle_ids;
    int idx = 0;
    for (int x = 0; x < 9; ++x) {
        for (int y = 0; y < 9; ++y) {
            for (int z = 0; z < 9; ++z) {
                VertexID v1 = idx;
                VertexID v2 = idx + 1;
                VertexID v3 = idx + 10;
                VertexID v4 = idx + 11;

                SimplexID t1 = complex.add_triangle(v1, v2, v3);
                SimplexID t2 = complex.add_triangle(v2, v3, v4);
                triangle_ids.push_back(t1);
                triangle_ids.push_back(t2);

                idx += 100;  // Move to next layer
            }
        }
    }

    // Build BVH
    BVHTree bvh(BVHBuildStrategy::SAH, 8, 20);
    bvh.build(triangle_ids, geometry, [&complex](SimplexID id) {
        return complex.get_simplex(id).vertices();
    });

    // Query range
    BoundingBox query_box(Point3D(2.0f, 2.0f, 2.0f), Point3D(4.0f, 4.0f, 4.0f));
    std::vector<SimplexID> results = bvh.query_range(query_box);

    // Verify results are within the box
    for (SimplexID id : results) {
        std::vector<VertexID> vertices = complex.get_simplex(id).vertices();
        for (VertexID vid : vertices) {
            auto pos = geometry.get_position(vid);
            if (pos) {
                assert(pos->x >= 2.0f && pos->x <= 4.0f);
                assert(pos->y >= 2.0f && pos->y <= 4.0f);
                assert(pos->z >= 2.0f && pos->z <= 4.0f);
            }
        }
    }

    std::cout << "  BVH range query: PASSED (found " << results.size() << " simplices)" << std::endl;
}

void test_bvh_nearest_neighbor() {
    std::cout << "Testing BVH nearest neighbor query..." << std::endl;

    SimplicialComplex complex;
    VertexGeometry geometry;

    // Create some vertices
    for (int i = 0; i < 50; ++i) {
        VertexID v = complex.add_vertex();
        float x = static_cast<float>(i % 10);
        float y = static_cast<float>((i / 10) % 5);
        float z = static_cast<float>(i / 50);
        geometry.set_position(v, x, y, z);
    }

    // Add triangles
    std::vector<SimplexID> triangle_ids;
    for (int i = 0; i < 20; ++i) {
        VertexID v1 = i;
        VertexID v2 = (i + 1) % 50;
        VertexID v3 = (i + 2) % 50;
        SimplexID tid = complex.add_triangle(v1, v2, v3);
        triangle_ids.push_back(tid);
    }

    // Build BVH
    BVHTree bvh(BVHBuildStrategy::SAH, 8, 20);
    bvh.build(triangle_ids, geometry, [&complex](SimplexID id) {
        return complex.get_simplex(id).vertices();
    });

    // Query nearest neighbors
    Point3D query_point(5.0f, 2.0f, 0.0f);
    std::vector<SimplexID> nearest = bvh.query_nearest(query_point, 3);

    assert(nearest.size() <= 3);
    assert(!nearest.empty());

    // Verify that results are actual simplices
    for (SimplexID id : nearest) {
        assert(complex.has_simplex(id));
    }

    std::cout << "  BVH nearest neighbor query: PASSED (found " << nearest.size() << " neighbors)" << std::endl;
}

void test_bvh_ray_query() {
    std::cout << "Testing BVH ray query..." << std::endl;

    SimplicialComplex complex;
    VertexGeometry geometry;

    // Create a simple cube
    // Bottom face
    VertexID v0 = complex.add_vertex(); geometry.set_position(v0, 0, 0, 0);
    VertexID v1 = complex.add_vertex(); geometry.set_position(v1, 1, 0, 0);
    VertexID v2 = complex.add_vertex(); geometry.set_position(v2, 1, 1, 0);
    VertexID v3 = complex.add_vertex(); geometry.set_position(v3, 0, 1, 0);
    // Top face
    VertexID v4 = complex.add_vertex(); geometry.set_position(v4, 0, 0, 1);
    VertexID v5 = complex.add_vertex(); geometry.set_position(v5, 1, 0, 1);
    VertexID v6 = complex.add_vertex(); geometry.set_position(v6, 1, 1, 1);
    VertexID v7 = complex.add_vertex(); geometry.set_position(v7, 0, 1, 1);

    // Add triangles
    std::vector<SimplexID> triangle_ids;
    triangle_ids.push_back(complex.add_triangle(v0, v1, v2));
    triangle_ids.push_back(complex.add_triangle(v0, v2, v3));
    triangle_ids.push_back(complex.add_triangle(v4, v5, v6));
    triangle_ids.push_back(complex.add_triangle(v4, v6, v7));
    triangle_ids.push_back(complex.add_triangle(v0, v1, v5));
    triangle_ids.push_back(complex.add_triangle(v0, v5, v4));
    triangle_ids.push_back(complex.add_triangle(v1, v2, v6));
    triangle_ids.push_back(complex.add_triangle(v1, v6, v5));
    triangle_ids.push_back(complex.add_triangle(v2, v3, v7));
    triangle_ids.push_back(complex.add_triangle(v2, v7, v6));
    triangle_ids.push_back(complex.add_triangle(v3, v0, v4));
    triangle_ids.push_back(complex.add_triangle(v3, v4, v7));

    // Build BVH
    BVHTree bvh(BVHBuildStrategy::SAH, 8, 20);
    bvh.build(triangle_ids, geometry, [&complex](SimplexID id) {
        return complex.get_simplex(id).vertices();
    });

    // Query ray through the cube
    Point3D origin(-1.0f, 0.5f, 0.5f);
    Point3D direction(1.0f, 0.0f, 0.0f);
    std::vector<SimplexID> results = bvh.query_ray(origin, direction);

    assert(!results.empty());
    std::cout << "  BVH ray query: PASSED (found " << results.size() << " intersections)" << std::endl;
}

void test_bvh_statistics() {
    std::cout << "Testing BVH statistics..." << std::endl;

    SimplicialComplex complex;
    VertexGeometry geometry;

    // Create many vertices
    for (int i = 0; i < 200; ++i) {
        VertexID v = complex.add_vertex();
        float x = static_cast<float>(rand()) / RAND_MAX * 100.0f;
        float y = static_cast<float>(rand()) / RAND_MAX * 100.0f;
        float z = static_cast<float>(rand()) / RAND_MAX * 100.0f;
        geometry.set_position(v, x, y, z);
    }

    // Add many triangles
    std::vector<SimplexID> triangle_ids;
    for (int i = 0; i < 100; ++i) {
        VertexID v1 = (i * 3) % 200;
        VertexID v2 = (i * 3 + 1) % 200;
        VertexID v3 = (i * 3 + 2) % 200;
        SimplexID tid = complex.add_triangle(v1, v2, v3);
        triangle_ids.push_back(tid);
    }

    // Build BVH
    BVHTree bvh(BVHBuildStrategy::SAH, 8, 20);
    bvh.build(triangle_ids, geometry, [&complex](SimplexID id) {
        return complex.get_simplex(id).vertices();
    });

    // Get statistics
    BVHTree::Statistics stats = bvh.get_statistics();

    assert(stats.total_nodes > 0);
    assert(stats.leaf_nodes > 0);
    assert(stats.internal_nodes > 0);
    assert(stats.max_tree_depth > 0);
    assert(stats.max_simplices_per_leaf > 0);

    std::cout << "  BVH Statistics:" << std::endl;
    std::cout << "    Total nodes: " << stats.total_nodes << std::endl;
    std::cout << "    Leaf nodes: " << stats.leaf_nodes << std::endl;
    std::cout << "    Internal nodes: " << stats.internal_nodes << std::endl;
    std::cout << "    Max depth: " << stats.max_tree_depth << std::endl;
    std::cout << "    Avg simplices/leaf: " << stats.avg_simplices_per_leaf << std::endl;
    std::cout << "    Max simplices/leaf: " << stats.max_simplices_per_leaf << std::endl;
    std::cout << "  BVH statistics: PASSED" << std::endl;
}

void test_bvh_build_strategies() {
    std::cout << "Testing different BVH build strategies..." << std::endl;

    SimplicialComplex complex;
    VertexGeometry geometry;

    // Create vertices
    for (int i = 0; i < 100; ++i) {
        VertexID v = complex.add_vertex();
        float x = static_cast<float>(rand()) / RAND_MAX * 50.0f;
        float y = static_cast<float>(rand()) / RAND_MAX * 50.0f;
        float z = static_cast<float>(rand()) / RAND_MAX * 50.0f;
        geometry.set_position(v, x, y, z);
    }

    // Add triangles
    std::vector<SimplexID> triangle_ids;
    for (int i = 0; i < 50; ++i) {
        VertexID v1 = i * 2 % 100;
        VertexID v2 = (i * 2 + 1) % 100;
        VertexID v3 = (i * 2 + 2) % 100;
        SimplexID tid = complex.add_triangle(v1, v2, v3);
        triangle_ids.push_back(tid);
    }

    // Test different strategies
    std::vector<BVHBuildStrategy> strategies = {
        BVHBuildStrategy::MEDIAN_SPLIT,
        BVHBuildStrategy::SAH,
        BVHBuildStrategy::MIDPOINT_SPLIT,
        BVHBuildStrategy::EQUAL_COUNTS
    };

    for (auto strategy : strategies) {
        BVHTree bvh(strategy, 8, 20);
        bvh.build(triangle_ids, geometry, [&complex](SimplexID id) {
            return complex.get_simplex(id).vertices();
        });

        assert(bvh.is_built());
        assert(bvh.simplex_count() == triangle_ids.size());

        // Test query
        BoundingBox query_box(Point3D(10.0f, 10.0f, 10.0f), Point3D(20.0f, 20.0f, 20.0f));
        std::vector<SimplexID> results = bvh.query_range(query_box);

        std::cout << "  Strategy " << static_cast<int>(strategy) << ": "
                  << bvh.node_count() << " nodes, "
                  << results.size() << " results" << std::endl;
    }

    std::cout << "  BVH build strategies: PASSED" << std::endl;
}

void test_vertex_geometry() {
    std::cout << "Testing VertexGeometry..." << std::endl;

    VertexGeometry geometry;

    // Test setting positions
    VertexID v1 = 1;
    VertexID v2 = 2;
    VertexID v3 = 3;

    geometry.set_position(v1, 1.0f, 2.0f, 3.0f);
    geometry.set_position(v2, Point3D(4.0f, 5.0f, 6.0f));
    geometry.set_position(v3, -1.0f, -2.0f, -3.0f);

    // Test getting positions
    auto pos1 = geometry.get_position(v1);
    assert(pos1.has_value());
    assert(pos1->x == 1.0f);
    assert(pos1->y == 2.0f);
    assert(pos1->z == 3.0f);

    auto pos2 = geometry.get_position(v2);
    assert(pos2.has_value());
    assert(pos2->x == 4.0f);

    auto pos3 = geometry.get_position(v3);
    assert(pos3.has_value());
    assert(pos3->x == -1.0f);

    // Test non-existent vertex
    auto pos_invalid = geometry.get_position(999);
    assert(!pos_invalid.has_value());

    // Test bounding box
    BoundingBox bbox = geometry.compute_bounding_box();
    assert(bbox.min.x == -1.0f);
    assert(bbox.max.x == 4.0f);

    // Test count
    assert(geometry.vertex_count() == 3);

    // Test remove
    geometry.remove_position(v2);
    assert(geometry.vertex_count() == 2);
    assert(!geometry.has_position(v2));

    std::cout << "  VertexGeometry: PASSED" << std::endl;
}

void benchmark_bvh() {
    std::cout << "\n=== BVH Performance Benchmark ===" << std::endl;

    SimplicialComplex complex;
    VertexGeometry geometry;

    const int NUM_VERTICES = 10000;
    const int NUM_TRIANGLES = 5000;

    std::cout << "Creating " << NUM_VERTICES << " vertices and "
              << NUM_TRIANGLES << " triangles..." << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    // Create vertices
    for (int i = 0; i < NUM_VERTICES; ++i) {
        VertexID v = complex.add_vertex();
        float x = static_cast<float>(rand()) / RAND_MAX * 1000.0f;
        float y = static_cast<float>(rand()) / RAND_MAX * 1000.0f;
        float z = static_cast<float>(rand()) / RAND_MAX * 1000.0f;
        geometry.set_position(v, x, y, z);
    }

    // Add triangles
    std::vector<SimplexID> triangle_ids;
    for (int i = 0; i < NUM_TRIANGLES; ++i) {
        VertexID v1 = rand() % NUM_VERTICES;
        VertexID v2 = rand() % NUM_VERTICES;
        VertexID v3 = rand() % NUM_VERTICES;
        SimplexID tid = complex.add_triangle(v1, v2, v3);
        triangle_ids.push_back(tid);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Geometry creation time: " << duration.count() << " ms" << std::endl;

    // Build BVH
    start = std::chrono::high_resolution_clock::now();
    BVHTree bvh(BVHBuildStrategy::SAH, 16, 30);
    bvh.build(triangle_ids, geometry, [&complex](SimplexID id) {
        return complex.get_simplex(id).vertices();
    });
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "BVH build time: " << duration.count() << " ms" << std::endl;

    // Get statistics
    BVHTree::Statistics stats = bvh.get_statistics();
    std::cout << "BVH Statistics:" << std::endl;
    std::cout << "  Total nodes: " << stats.total_nodes << std::endl;
    std::cout << "  Leaf nodes: " << stats.leaf_nodes << std::endl;
    std::cout << "  Max depth: " << stats.max_tree_depth << std::endl;

    // Benchmark range queries
    const int NUM_QUERIES = 1000;
    std::cout << "\nRunning " << NUM_QUERIES << " range queries..." << std::endl;

    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_QUERIES; ++i) {
        float x = static_cast<float>(rand()) / RAND_MAX * 900.0f;
        float y = static_cast<float>(rand()) / RAND_MAX * 900.0f;
        float z = static_cast<float>(rand()) / RAND_MAX * 900.0f;
        BoundingBox query_box(Point3D(x, y, z), Point3D(x + 50, y + 50, z + 50));
        std::vector<SimplexID> results = bvh.query_range(query_box);
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Total query time: " << duration.count() << " ms" << std::endl;
    std::cout << "Average query time: " << static_cast<double>(duration.count()) / NUM_QUERIES << " ms" << std::endl;

    // Benchmark nearest neighbor queries
    std::cout << "\nRunning " << NUM_QUERIES << " nearest neighbor queries..." << std::endl;
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_QUERIES; ++i) {
        float x = static_cast<float>(rand()) / RAND_MAX * 1000.0f;
        float y = static_cast<float>(rand()) / RAND_MAX * 1000.0f;
        float z = static_cast<float>(rand()) / RAND_MAX * 1000.0f;
        Point3D point(x, y, z);
        std::vector<SimplexID> results = bvh.query_nearest(point, 10);
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Total query time: " << duration.count() << " ms" << std::endl;
    std::cout << "Average query time: " << static_cast<double>(duration.count()) / NUM_QUERIES << " ms" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "      BVH (Bounding Volume Hierarchy)   " << std::endl;
    std::cout << "           Test Suite                   " << std::endl;
    std::cout << "========================================" << std::endl;

    try {
        test_vertex_geometry();
        test_bvh_construction();
        test_bvh_range_query();
        test_bvh_nearest_neighbor();
        test_bvh_ray_query();
        test_bvh_statistics();
        test_bvh_build_strategies();
        benchmark_bvh();

        std::cout << "\n========================================" << std::endl;
        std::cout << "         ALL TESTS PASSED!              " << std::endl;
        std::cout << "========================================" << std::endl;

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
}
