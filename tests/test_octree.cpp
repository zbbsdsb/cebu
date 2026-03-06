#include "cebu/octree.h"
#include "cebu/simplicial_complex.h"
#include <iostream>
#include <cassert>
#include <random>
#include <chrono>

using namespace cebu;

void test_octree_construction() {
    std::cout << "Testing Octree construction..." << std::endl;

    SimplicialComplex complex;
    VertexGeometry geometry;

    // Create vertices with geometry
    for (int i = 0; i < 100; ++i) {
        VertexID v = complex.add_vertex();
        float x = static_cast<float>(rand()) / RAND_MAX * 10.0f;
        float y = static_cast<float>(rand()) / RAND_MAX * 10.0f;
        float z = static_cast<float>(rand()) / RAND_MAX * 10.0f;
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

    // Build Octree
    Octree octree(10, 8);
    octree.build(triangle_ids, geometry, [&complex](SimplexID id) {
        return complex.get_simplex(id).vertices();
    });

    // Check Octree is built
    assert(octree.is_built());
    assert(octree.node_count() > 0);
    assert(octree.simplex_count() == triangle_ids.size());

    std::cout << "  Octree construction: PASSED" << std::endl;
}

void test_octree_range_query() {
    std::cout << "Testing Octree range query..." << std::endl;

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

                idx += 100;
            }
            idx += 10;
        }
    }

    // Build Octree
    Octree octree(8, 16);
    octree.build(triangle_ids, geometry, [&complex](SimplexID id) {
        return complex.get_simplex(id).vertices();
    });

    // Query range
    BoundingBox query_box(Point3D(2.0f, 2.0f, 2.0f), Point3D(4.0f, 4.0f, 4.0f));
    std::vector<SimplexID> results = octree.query_range(query_box);

    // Verify results
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

    std::cout << "  Octree range query: PASSED (found " << results.size() << " simplices)" << std::endl;
}

void test_octree_point_query() {
    std::cout << "Testing Octree point query..." << std::endl;

    SimplicialComplex complex;
    VertexGeometry geometry;

    // Create a cube
    VertexID v0 = complex.add_vertex(); geometry.set_position(v0, 0, 0, 0);
    VertexID v1 = complex.add_vertex(); geometry.set_position(v1, 1, 0, 0);
    VertexID v2 = complex.add_vertex(); geometry.set_position(v2, 0, 1, 0);
    VertexID v3 = complex.add_vertex(); geometry.set_position(v3, 1, 1, 0);

    // Add triangles
    std::vector<SimplexID> triangle_ids;
    triangle_ids.push_back(complex.add_triangle(v0, v1, v2));
    triangle_ids.push_back(complex.add_triangle(v1, v2, v3));

    // Build Octree
    Octree octree(5, 8);
    octree.build(triangle_ids, geometry, [&complex](SimplexID id) {
        return complex.get_simplex(id).vertices();
    });

    // Query point
    Point3D query_point(0.5f, 0.5f, 0.0f);
    std::vector<SimplexID> results = octree.query_point(query_point);

    // Should find triangles containing the point
    assert(!results.empty());

    std::cout << "  Octree point query: PASSED (found " << results.size() << " simplices)" << std::endl;
}

void test_octree_nearest_neighbor() {
    std::cout << "Testing Octree nearest neighbor query..." << std::endl;

    SimplicialComplex complex;
    VertexGeometry geometry;

    // Create vertices
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

    // Build Octree
    Octree octree(8, 16);
    octree.build(triangle_ids, geometry, [&complex](SimplexID id) {
        return complex.get_simplex(id).vertices();
    });

    // Query nearest neighbors
    Point3D query_point(5.0f, 2.0f, 0.0f);
    std::vector<SimplexID> nearest = octree.query_nearest(query_point, 3);

    assert(nearest.size() <= 3);
    assert(!nearest.empty());

    // Verify results are actual simplices
    for (SimplexID id : nearest) {
        assert(complex.has_simplex(id));
    }

    std::cout << "  Octree nearest neighbor query: PASSED (found " << nearest.size() << " neighbors)" << std::endl;
}

void test_octree_statistics() {
    std::cout << "Testing Octree statistics..." << std::endl;

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

    // Build Octree
    Octree octree(12, 16);
    octree.build(triangle_ids, geometry, [&complex](SimplexID id) {
        return complex.get_simplex(id).vertices();
    });

    // Get statistics
    Octree::Statistics stats = octree.get_statistics();

    assert(stats.total_nodes > 0);
    assert(stats.leaf_nodes > 0);
    assert(stats.internal_nodes > 0);
    assert(stats.max_tree_depth > 0);
    assert(stats.max_simplices_per_leaf > 0);

    std::cout << "  Octree Statistics:" << std::endl;
    std::cout << "    Total nodes: " << stats.total_nodes << std::endl;
    std::cout << "    Leaf nodes: " << stats.leaf_nodes << std::endl;
    std::cout << "    Internal nodes: " << stats.internal_nodes << std::endl;
    std::cout << "    Max depth: " << stats.max_tree_depth << std::endl;
    std::cout << "    Avg simplices/leaf: " << stats.avg_simplices_per_leaf << std::endl;
    std::cout << "    Max simplices/leaf: " << stats.max_simplices_per_leaf << std::endl;
    std::cout << "    Empty nodes: " << stats.empty_nodes << std::endl;
    std::cout << "  Octree statistics: PASSED" << std::endl;
}

void test_octree_depth_control() {
    std::cout << "Testing Octree depth control..." << std::endl;

    SimplicialComplex complex;
    VertexGeometry geometry;

    // Create grid of vertices
    for (int i = 0; i < 1000; ++i) {
        VertexID v = complex.add_vertex();
        float x = static_cast<float>(i % 10);
        float y = static_cast<float>((i / 10) % 10);
        float z = static_cast<float>(i / 100);
        geometry.set_position(v, x, y, z);
    }

    // Add triangles
    std::vector<SimplexID> triangle_ids;
    for (int i = 0; i < 200; ++i) {
        VertexID v1 = (i * 5) % 1000;
        VertexID v2 = (i * 5 + 1) % 1000;
        VertexID v3 = (i * 5 + 2) % 1000;
        SimplexID tid = complex.add_triangle(v1, v2, v3);
        triangle_ids.push_back(tid);
    }

    // Build Octree with limited depth
    size_t max_depth = 5;
    Octree octree(max_depth, 8);
    octree.build(triangle_ids, geometry, [&complex](SimplexID id) {
        return complex.get_simplex(id).vertices();
    });

    // Get statistics
    Octree::Statistics stats = octree.get_statistics();

    // Verify depth limit is respected
    assert(stats.max_tree_depth <= max_depth);

    std::cout << "  Octree depth control: PASSED (max depth: " << stats.max_tree_depth << ")" << std::endl;
}

void benchmark_octree() {
    std::cout << "\n=== Octree Performance Benchmark ===" << std::endl;

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

    // Build Octree
    start = std::chrono::high_resolution_clock::now();
    Octree octree(12, 16);
    octree.build(triangle_ids, geometry, [&complex](SimplexID id) {
        return complex.get_simplex(id).vertices();
    });
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Octree build time: " << duration.count() << " ms" << std::endl;

    // Get statistics
    Octree::Statistics stats = octree.get_statistics();
    std::cout << "Octree Statistics:" << std::endl;
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
        std::vector<SimplexID> results = octree.query_range(query_box);
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
        std::vector<SimplexID> results = octree.query_nearest(point, 10);
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Total query time: " << duration.count() << " ms" << std::endl;
    std::cout << "Average query time: " << static_cast<double>(duration.count()) / NUM_QUERIES << " ms" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "         Octree Test Suite           " << std::endl;
    std::cout << "========================================" << std::endl;

    try {
        test_octree_construction();
        test_octree_range_query();
        test_octree_point_query();
        test_octree_nearest_neighbor();
        test_octree_statistics();
        test_octree_depth_control();
        benchmark_octree();

        std::cout << "\n========================================" << std::endl;
        std::cout << "         ALL TESTS PASSED!          " << std::endl;
        std::cout << "========================================" << std::endl;

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
}
