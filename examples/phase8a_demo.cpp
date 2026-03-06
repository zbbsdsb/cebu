/**
 * Phase 8a Demo: BVH (Bounding Volume Hierarchy) Demo
 *
 * This demo shows how to use BVH spatial indexing to accelerate spatial queries
 */

#include "cebu/bvh.h"
#include "cebu/simplicial_complex.h"
#include <iostream>
#include <cmath>

using namespace cebu;

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "   Phase 8a: BVH Spatial Index Demo   " << std::endl;
    std::cout << "========================================" << std::endl;

    // 1. Create simplicial complex
    SimplicialComplex complex;
    std::cout << "\n1. Creating simplicial complex..." << std::endl;

    // 2. Create vertex geometry manager
    VertexGeometry geometry;
    std::cout << "2. Creating vertex geometry manager..." << std::endl;

    // 3. Add vertices (create a simple grid)
    const int GRID_SIZE = 10;
    for (int x = 0; x < GRID_SIZE; ++x) {
        for (int y = 0; y < GRID_SIZE; ++y) {
            for (int z = 0; z < GRID_SIZE; ++z) {
                VertexID v = complex.add_vertex();
                geometry.set_position(v,
                    static_cast<float>(x),
                    static_cast<float>(y),
                    static_cast<float>(z));
            }
        }
    }
    std::cout << "   Added " << complex.vertex_count() << " vertices" << std::endl;

    // 4. Add triangles
    std::vector<SimplexID> triangle_ids;
    int idx = 0;
    for (int x = 0; x < GRID_SIZE - 1; ++x) {
        for (int y = 0; y < GRID_SIZE - 1; ++y) {
            for (int z = 0; z < GRID_SIZE - 1; ++z) {
                VertexID v1 = idx;
                VertexID v2 = idx + 1;
                VertexID v3 = idx + GRID_SIZE;
                VertexID v4 = idx + GRID_SIZE + 1;

                // Add two triangles to form a quad face
                SimplexID t1 = complex.add_triangle(v1, v2, v3);
                SimplexID t2 = complex.add_triangle(v2, v3, v4);
                triangle_ids.push_back(t1);
                triangle_ids.push_back(t2);

                idx += GRID_SIZE * GRID_SIZE;
            }
            idx += GRID_SIZE;
        }
    }
    std::cout << "   Added " << triangle_ids.size() << " triangles" << std::endl;

    // 5. Build BVH
    std::cout << "\n3. Building BVH..." << std::endl;
    BVHTree bvh(BVHBuildStrategy::SAH, 16, 20);

    auto start = std::chrono::high_resolution_clock::now();
    bvh.build(triangle_ids, geometry, [&complex](SimplexID id) {
        return complex.get_simplex(id).vertices();
    });
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "   BVH built, time: " << duration.count() << " ms" << std::endl;
    std::cout << "   Nodes: " << bvh.node_count() << std::endl;
    std::cout << "   Simplices: " << bvh.simplex_count() << std::endl;

    // 6. Get BVH statistics
    std::cout << "\n4. BVH Statistics:" << std::endl;
    BVHTree::Statistics stats = bvh.get_statistics();
    std::cout << "   Total nodes: " << stats.total_nodes << std::endl;
    std::cout << "   Leaf nodes: " << stats.leaf_nodes << std::endl;
    std::cout << "   Internal nodes: " << stats.internal_nodes << std::endl;
    std::cout << "   Max depth: " << stats.max_tree_depth << std::endl;
    std::cout << "   Avg simplices/leaf: " << stats.avg_simplices_per_leaf << std::endl;
    std::cout << "   Max simplices/leaf: " << stats.max_simplices_per_leaf << std::endl;

    // 7. Range query
    std::cout << "\n5. Range query:" << std::endl;
    BoundingBox query_box(Point3D(2.0f, 2.0f, 2.0f), Point3D(4.0f, 4.0f, 4.0f));
    std::cout << "   Query bbox: [" << query_box.min.x << "," << query_box.min.y << "," << query_box.min.z
              << "] to [" << query_box.max.x << "," << query_box.max.y << "," << query_box.max.z << "]" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    std::vector<SimplexID> results = bvh.query_range(query_box);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << "   Found " << results.size() << " triangles" << std::endl;
    std::cout << "   Query time: " << duration.count() << " μs" << std::endl;

    // 8. Nearest neighbor query
    std::cout << "\n6. Nearest neighbor query:" << std::endl;
    Point3D query_point(5.0f, 5.0f, 5.0f);
    std::cout << "   Query point: (" << query_point.x << "," << query_point.y << "," << query_point.z << ")" << std::endl;
    std::cout << "   Finding 5 nearest triangles..." << std::endl;

    start = std::chrono::high_resolution_clock::now();
    std::vector<SimplexID> nearest = bvh.query_nearest(query_point, 5);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << "   Found " << nearest.size() << " nearest neighbors" << std::endl;
    std::cout << "   Query time: " << duration.count() << " μs" << std::endl;

    if (!nearest.empty()) {
        std::cout << "   Center of nearest triangle: ";
        auto vertices = complex.get_simplex(nearest[0]).vertices();
        Point3D centroid;
        for (VertexID vid : vertices) {
            auto pos = geometry.get_position(vid);
            if (pos) {
                centroid = centroid + *pos;
            }
        }
        centroid = centroid * (1.0f / vertices.size());
        std::cout << "(" << centroid.x << "," << centroid.y << "," << centroid.z << ")" << std::endl;
    }

    // 9. Ray query
    std::cout << "\n7. Ray query:" << std::endl;
    Point3D ray_origin(-1.0f, 5.0f, 5.0f);
    Point3D ray_direction(1.0f, 0.0f, 0.0f);
    std::cout << "   Ray origin: (" << ray_origin.x << "," << ray_origin.y << "," << ray_origin.z << ")" << std::endl;
    std::cout << "   Ray direction: (" << ray_direction.x << "," << ray_direction.y << "," << ray_direction.z << ")" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    std::vector<SimplexID> ray_results = bvh.query_ray(ray_origin, ray_direction);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << "   Found " << ray_results.size() << " intersecting triangles" << std::endl;
    std::cout << "   Query time: " << duration.count() << " μs" << std::endl;

    // 10. Incremental update demo
    std::cout << "\n8. Incremental update:" << std::endl;
    VertexID new_v1 = complex.add_vertex();
    VertexID new_v2 = complex.add_vertex();
    VertexID new_v3 = complex.add_vertex();
    geometry.set_position(new_v1, 10.5f, 10.5f, 10.5f);
    geometry.set_position(new_v2, 11.0f, 10.5f, 10.5f);
    geometry.set_position(new_v3, 10.5f, 11.0f, 10.5f);

    SimplexID new_triangle = complex.add_triangle(new_v1, new_v2, new_v3);
    std::cout << "   Added new triangle, ID: " << new_triangle << std::endl;

    bvh.add_simplex(new_triangle);
    std::cout << "   Updated simplex count: " << bvh.simplex_count() << std::endl;

    // 11. Compare different build strategies
    std::cout << "\n9. Comparing different build strategies:" << std::endl;
    std::vector<std::pair<BVHBuildStrategy, std::string>> strategies = {
        {BVHBuildStrategy::MEDIAN_SPLIT, "Median split"},
        {BVHBuildStrategy::SAH, "Surface Area Heuristic"},
        {BVHBuildStrategy::MIDPOINT_SPLIT, "Midpoint split"},
        {BVHBuildStrategy::EQUAL_COUNTS, "Equal counts"}
    };

    for (auto [strategy, name] : strategies) {
        BVHTree test_bvh(strategy, 16, 20);

        start = std::chrono::high_resolution_clock::now();
        test_bvh.build(triangle_ids, geometry, [&complex](SimplexID id) {
            return complex.get_simplex(id).vertices();
        });
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        auto test_stats = test_bvh.get_statistics();
        std::cout << "   " << name << ":" << std::endl;
        std::cout << "     Build time: " << duration.count() << " ms" << std::endl;
        std::cout << "     Nodes: " << test_bvh.node_count() << std::endl;
        std::cout << "     Max depth: " << test_stats.max_tree_depth << std::endl;
    }

    std::cout << "\n========================================" << std::endl;
    std::cout << "   Demo completed!                       " << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
