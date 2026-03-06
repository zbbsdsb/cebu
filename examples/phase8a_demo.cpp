/**
 * Phase 8a Demo: BVH (Bounding Volume Hierarchy) 演示
 *
 * 本示例展示如何使用 BVH 空间索引来加速空间查询
 */

#include "cebu/bvh.h"
#include "cebu/simplicial_complex.h"
#include <iostream>
#include <cmath>

using namespace cebu;

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "   Phase 8a: BVH 空间索引演示          " << std::endl;
    std::cout << "========================================" << std::endl;

    // 1. 创建单纯复形
    SimplicialComplex complex;
    std::cout << "\n1. 创建单纯复形..." << std::endl;

    // 2. 创建顶点几何管理器
    VertexGeometry geometry;
    std::cout << "2. 创建顶点几何管理器..." << std::endl;

    // 3. 添加一些顶点（创建一个简单的网格）
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
    std::cout << "   添加了 " << complex.vertex_count() << " 个顶点" << std::endl;

    // 4. 添加一些三角形
    std::vector<SimplexID> triangle_ids;
    int idx = 0;
    for (int x = 0; x < GRID_SIZE - 1; ++x) {
        for (int y = 0; y < GRID_SIZE - 1; ++y) {
            for (int z = 0; z < GRID_SIZE - 1; ++z) {
                VertexID v1 = idx;
                VertexID v2 = idx + 1;
                VertexID v3 = idx + GRID_SIZE;
                VertexID v4 = idx + GRID_SIZE + 1;

                // 添加两个三角形形成一个四边形面
                SimplexID t1 = complex.add_triangle(v1, v2, v3);
                SimplexID t2 = complex.add_triangle(v2, v3, v4);
                triangle_ids.push_back(t1);
                triangle_ids.push_back(t2);

                idx += GRID_SIZE * GRID_SIZE;
            }
            idx += GRID_SIZE;
        }
    }
    std::cout << "   添加了 " << triangle_ids.size() << " 个三角形" << std::endl;

    // 5. 构建 BVH
    std::cout << "\n3. 构建 BVH..." << std::endl;
    BVHTree bvh(BVHBuildStrategy::SAH, 16, 20);

    auto start = std::chrono::high_resolution_clock::now();
    bvh.build(triangle_ids, geometry, [&complex](SimplexID id) {
        return complex.get_simplex(id).vertices();
    });
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "   BVH 构建完成，耗时: " << duration.count() << " ms" << std::endl;
    std::cout << "   节点数: " << bvh.node_count() << std::endl;
    std::cout << "   单纯形数: " << bvh.simplex_count() << std::endl;

    // 6. 获取 BVH 统计信息
    std::cout << "\n4. BVH 统计信息:" << std::endl;
    BVHTree::Statistics stats = bvh.get_statistics();
    std::cout << "   总节点数: " << stats.total_nodes << std::endl;
    std::cout << "   叶节点数: " << stats.leaf_nodes << std::endl;
    std::cout << "   内部节点数: " << stats.internal_nodes << std::endl;
    std::cout << "   最大深度: " << stats.max_tree_depth << std::endl;
    std::cout << "   平均每叶节点单纯形数: " << stats.avg_simplices_per_leaf << std::endl;
    std::cout << "   最大每叶节点单纯形数: " << stats.max_simplices_per_leaf << std::endl;

    // 7. 范围查询
    std::cout << "\n5. 范围查询:" << std::endl;
    BoundingBox query_box(Point3D(2.0f, 2.0f, 2.0f), Point3D(4.0f, 4.0f, 4.0f));
    std::cout << "   查询包围盒: [" << query_box.min.x << "," << query_box.min.y << "," << query_box.min.z
              << "] 到 [" << query_box.max.x << "," << query_box.max.y << "," << query_box.max.z << "]";

    start = std::chrono::high_resolution_clock::now();
    std::vector<SimplexID> results = bvh.query_range(query_box);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << "   找到 " << results.size() << " 个三角形" << std::endl;
    std::cout << "   查询时间: " << duration.count() << " μs" << std::endl;

    // 8. 最近邻查询
    std::cout << "\n6. 最近邻查询:" << std::endl;
    Point3D query_point(5.0f, 5.0f, 5.0f);
    std::cout << "   查询点: (" << query_point.x << "," << query_point.y << "," << query_point.z << ")" << std::endl;
    std::cout << "   查找最近的 5 个三角形..." << std::endl;

    start = std::chrono::high_resolution_clock::now();
    std::vector<SimplexID> nearest = bvh.query_nearest(query_point, 5);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << "   找到 " << nearest.size() << " 个最近邻" << std::endl;
    std::cout << "   查询时间: " << duration.count() << " μs" << std::endl;

    if (!nearest.empty()) {
        std::cout << "   最近三角形的中心点: ";
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

    // 9. 射线查询
    std::cout << "\n7. 射线查询:" << std::endl;
    Point3D ray_origin(-1.0f, 5.0f, 5.0f);
    Point3D ray_direction(1.0f, 0.0f, 0.0f);
    std::cout << "   射线起点: (" << ray_origin.x << "," << ray_origin.y << "," << ray_origin.z << ")" << std::endl;
    std::cout << "   射线方向: (" << ray_direction.x << "," << ray_direction.y << "," << ray_direction.z << ")" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    std::vector<SimplexID> ray_results = bvh.query_ray(ray_origin, ray_direction);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << "   找到 " << ray_results.size() << " 个相交三角形" << std::endl;
    std::cout << "   查询时间: " << duration.count() << " μs" << std::endl;

    // 10. 增量更新演示
    std::cout << "\n8. 增量更新:" << std::endl;
    VertexID new_v1 = complex.add_vertex();
    VertexID new_v2 = complex.add_vertex();
    VertexID new_v3 = complex.add_vertex();
    geometry.set_position(new_v1, 10.5f, 10.5f, 10.5f);
    geometry.set_position(new_v2, 11.0f, 10.5f, 10.5f);
    geometry.set_position(new_v3, 10.5f, 11.0f, 10.5f);

    SimplexID new_triangle = complex.add_triangle(new_v1, new_v2, new_v3);
    std::cout << "   添加新三角形，ID: " << new_triangle << std::endl;

    bvh.add_simplex(new_triangle);
    std::cout << "   更新后的单纯形数: " << bvh.simplex_count() << std::endl;

    // 11. 比较不同构建策略
    std::cout << "\n9. 比较不同构建策略:" << std::endl;
    std::vector<std::pair<BVHBuildStrategy, std::string>> strategies = {
        {BVHBuildStrategy::MEDIAN_SPLIT, "中位数分割"},
        {BVHBuildStrategy::SAH, "表面积启发式"},
        {BVHBuildStrategy::MIDPOINT_SPLIT, "中点分割"},
        {BVHBuildStrategy::EQUAL_COUNTS, "等量分割"}
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
        std::cout << "     构建时间: " << duration.count() << " ms" << std::endl;
        std::cout << "     节点数: " << test_bvh.node_count() << std::endl;
        std::cout << "     最大深度: " << test_stats.max_tree_depth << std::endl;
    }

    std::cout << "\n========================================" << std::endl;
    std::cout << "   演示完成！                          " << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
