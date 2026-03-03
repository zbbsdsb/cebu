#pragma once

#include <vector>
#include <cstdint>
#include <unordered_set>

namespace cebu {

using SimplexID = uint64_t;
using VertexID = uint32_t;

/**
 * @brief 单纯形类，表示一个单纯形（顶点、边、三角形等）
 *
 * 单纯形是组合拓扑的基本元素：
 * - 0-单纯形：顶点
 * - 1-单纯形：边
 * - 2-单纯形：三角形
 * - k-单纯形：由 k+1 个顶点构成的凸包
 */
class Simplex {
public:
    /**
     * @brief 构造函数
     * @param vertices 构成此单纯形的顶点 ID 列表
     * @param id 单纯形的唯一 ID
     */
    explicit Simplex(const std::vector<VertexID>& vertices, SimplexID id);

    /**
     * @brief 获取单纯形的 ID
     */
    SimplexID id() const { return id_; }

    /**
     * @brief 获取单纯形的维度
     * 0-单纯形：顶点（1个顶点）
     * 1-单纯形：边（2个顶点）
     * 2-单纯形：三角形（3个顶点）
     */
    size_t dimension() const { return vertices_.size() - 1; }

    /**
     * @brief 获取构成此单纯形的顶点列表
     */
    const std::vector<VertexID>& vertices() const { return vertices_; }

    /**
     * @brief 获取相邻单纯形的 ID 集合
     */
    const std::unordered_set<SimplexID>& neighbors() const { return neighbors_; }

    /**
     * @brief 添加相邻单纯形
     * @param neighbor_id 相邻单纯形的 ID
     */
    void add_neighbor(SimplexID neighbor_id);

    /**
     * @brief 移除相邻单纯形
     * @param neighbor_id 要移除的相邻单纯形 ID
     */
    void remove_neighbor(SimplexID neighbor_id);

    /**
     * @brief 检查是否与给定的单纯形相邻
     * @param neighbor_id 要检查的单纯形 ID
     */
    bool has_neighbor(SimplexID neighbor_id) const;

private:
    SimplexID id_;
    std::vector<VertexID> vertices_;
    std::unordered_set<SimplexID> neighbors_;
};

} // namespace cebu
