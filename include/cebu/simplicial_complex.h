#pragma once

#include "cebu/simplex.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>

namespace cebu {

/**
 * @brief 静态单纯复形类
 *
 * 单纯复形是由单纯形组成的集合，满足：
 * 1. 如果单纯形 A 在复形中，则 A 的所有面也在复形中
 * 2. 两个单纯形的交集是它们共同的某个面（或为空）
 *
 * 此实现提供静态复形，可以添加单纯形但不支持删除。
 */
class SimplicialComplex {
public:
    /**
     * @brief 构造函数
     */
    SimplicialComplex();

    /**
     * @brief 添加一个顶点（0-单纯形）
     * @return 新顶点的 ID
     */
    VertexID add_vertex();

    /**
     * @brief 添加一个边（1-单纯形）
     * @param v1 第一个顶点 ID
     * @param v2 第二个顶点 ID
     * @return 新边的单纯形 ID
     * @throw std::invalid_argument 如果顶点不存在
     */
    SimplexID add_edge(VertexID v1, VertexID v2);

    /**
     * @brief 添加一个三角形（2-单纯形）
     * @param v1 第一个顶点 ID
     * @param v2 第二个顶点 ID
     * @param v3 第三个顶点 ID
     * @return 新三角形的单纯形 ID
     * @throw std::invalid_argument 如果顶点不存在
     */
    SimplexID add_triangle(VertexID v1, VertexID v2, VertexID v3);

    /**
     * @brief 添加一个 k-单纯形
     * @param vertices 构成单纯形的顶点列表
     * @return 新单纯形的 ID
     * @throw std::invalid_argument 如果顶点不存在或数量不足
     */
    SimplexID add_simplex(const std::vector<VertexID>& vertices);

    /**
     * @brief 获取所有顶点的 ID
     */
    const std::vector<VertexID>& get_vertices() const { return vertices_; }

    /**
     * @brief 获取所有单纯形
     */
    const std::unordered_map<SimplexID, Simplex>& get_simplices() const {
        return simplices_;
    }

    /**
     * @brief 获取指定维度的单纯形 ID 列表
     * @param dimension 维度
     */
    std::vector<SimplexID> get_simplices_of_dimension(size_t dimension) const;

    /**
     * @brief 检查单纯形是否存在
     * @param id 单纯形 ID
     */
    bool has_simplex(SimplexID id) const {
        return simplices_.find(id) != simplices_.end();
    }

    /**
     * @brief 获取指定 ID 的单纯形
     * @param id 单纯形 ID
     * @return 单纯形的引用
     * @throw std::out_of_range 如果单纯形不存在
     */
    const Simplex& get_simplex(SimplexID id) const;

    /**
     * @brief 获取包含指定顶点的所有单纯形
     * @param vertex_id 顶点 ID
     */
    std::vector<SimplexID> get_simplices_containing_vertex(VertexID vertex_id) const;

    /**
     * @brief 获取与指定单纯形相邻的所有单纯形
     * @param simplex_id 单纯形 ID
     */
    std::vector<SimplexID> get_adjacent_simplices(SimplexID simplex_id) const;

    /**
     * @brief 获取单纯形的所有面（维度低 1 的边界单纯形）
     * @param simplex_id 单纯形 ID
     */
    std::vector<SimplexID> get_facets(SimplexID simplex_id) const;

    /**
     * @brief 获取单纯形数量
     */
    size_t simplex_count() const { return simplices_.size(); }

    /**
     * @brief 获取顶点数量
     */
    size_t vertex_count() const { return vertices_.size(); }

private:
    std::vector<VertexID> vertices_;
    std::unordered_map<SimplexID, Simplex> simplices_;
    SimplexID next_simplex_id_;

    // 顶点到包含它的单纯形的映射
    std::unordered_map<VertexID, std::unordered_set<SimplexID>> vertex_to_simplices_;

    /**
     * @brief 根据顶点列表计算单纯形的唯一标识（用于查重）
     */
    std::string compute_hash(const std::vector<VertexID>& vertices) const;
};

} // namespace cebu
