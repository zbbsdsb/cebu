// Copyright (c) 2026 CeaserZhao (Oasis Company)
// SPDX-License-Identifier: MIT
#pragma once

#include <cebu/simplicial_complex.h>
#include <cebu/equivalence_classes.h>
#include <nlohmann/json.hpp>
#include <vector>
#include <functional>
#include <chrono>
#include <string>

namespace cebu {

/**
 * @brief 变化类型枚举
 */
enum class ChangeType {
    SIMPLEX_ADDED,
    SIMPLEX_REMOVED,
    LABEL_CHANGED,
    EQUIVALENCE_CHANGED,
    TOPOLOGY_CHANGED
};

/**
 * @brief 变化记录
 */
struct Change {
    ChangeType type;
    SimplexID simplex_id;
    size_t dimension;
    std::vector<VertexID> vertices;
    std::chrono::system_clock::time_point timestamp;

    // For label changes
    double old_label;
    double new_label;

    // For equivalence changes (using SimplexID as class representative)
    SimplexID old_equivalence;
    SimplexID new_equivalence;

    Change(ChangeType t, SimplexID id)
        : type(t), simplex_id(id), dimension(0), timestamp(std::chrono::system_clock::now()),
          old_label(0.0), new_label(0.0), old_equivalence(0), new_equivalence(0) {}
};

/**
 * @brief 变化追踪�? * 
 * 追踪复形的所有变化，用于增量序列化和版本控制
 */
class ChangeTracker {
public:
    /**
     * @brief 构造函�?     */
    ChangeTracker();

    /**
     * @brief 析构函数
     */
    ~ChangeTracker() = default;

    /**
     * @brief 追踪单纯形添�?     * @param simplex_id 单纯�?ID
     * @param dimension 单纯形维�?     * @param vertices 顶点列表
     */
    void track_simplex_added(
        SimplexID simplex_id,
        size_t dimension,
        const std::vector<VertexID>& vertices);

    /**
     * @brief 追踪单纯形删�?     * @param simplex_id 单纯�?ID
     * @param dimension 单纯形维�?     * @param vertices 顶点列表
     */
    void track_simplex_removed(
        SimplexID simplex_id,
        size_t dimension,
        const std::vector<VertexID>& vertices);

    /**
     * @brief 追历标签变化
     * @param simplex_id 单纯�?ID
     * @param old_label 旧标签�?     * @param new_label 新标签�?     */
    void track_label_changed(
        SimplexID simplex_id,
        double old_label,
        double new_label);

    /**
     * @brief 追历等价类变�?     * @param equivalence_id 等价�?ID
     * @param old_equivalence 旧等价类
     * @param new_equivalence 新等价类
     */
    void track_equivalence_changed(
        SimplexID equivalence_id,
        SimplexID old_equivalence,
        SimplexID new_equivalence);

    /**
     * @brief 追历拓扑变化
     */
    void track_topology_changed();

    /**
     * @brief 重置追踪�?     */
    void reset();

    /**
     * @brief 获取所有变�?     * @return 变化列表
     */
    const std::vector<Change>& get_changes() const;

    /**
     * @brief 获取特定类型的变�?     * @param type 变化类型
     * @return 变化列表
     */
    std::vector<Change> get_changes_by_type(ChangeType type) const;

    /**
     * @brief 获取特定单纯形的变化
     * @param simplex_id 单单纯形 ID
     * @return 变化列表
     */
    std::vector<Change> get_changes_by_simplex(SimplexID simplex_id) const;

    /**
     * @brief 获取变化数量
     * @return 变化数量
     */
    size_t get_change_count() const;

    /**
     * @brief 检查是否有变化
     * @return 是否有变�?     */
    bool has_changes() const;

    /**
     * @brief 获取时间范围内的变化
     * @param start 开始时�?     * @param end 结束时间
     * @return 变化列表
     */
    std::vector<Change> get_changes_in_time_range(
        const std::chrono::system_clock::time_point& start,
        const std::chrono::system_clock::time_point& end) const;

    /**
     * @brief 导出变化�?JSON
     * @return JSON 对象
     */
    nlohmann::json to_json() const;

    /**
     * @brief �?JSON 导入变化
     * @param j JSON 对象
     */
    void from_json(const nlohmann::json& j);

    /**
     * @brief 保存变化到文�?     * @param filename 文件�?     * @return 是否成功
     */
    bool save_to_file(const std::string& filename) const;

    /**
     * @brief 从文件加载变�?     * @param filename 文件�?     * @return 是否成功
     */
    bool load_from_file(const std::string& filename);

private:
    std::vector<Change> changes_;
    
    /**
     * @brief 添加变化
     * @param change 变化
     */
    void add_change(const Change& change);
};

/**
 * @brief 增量保存结果
 */
struct DeltaSaveResult {
    bool success;
    std::string filename;
    size_t original_size;
    size_t compressed_size;
    double compression_ratio;
    size_t change_count;
    std::chrono::system_clock::time_point timestamp;
    std::string error_message;
};

/**
 * @brief 增量加载结果
 */
struct DeltaLoadResult {
    bool success;
    size_t change_count;
    std::chrono::system_clock::time_point timestamp;
    std::string error_message;
};

} // namespace cebu
