#pragma once

#include <cebu/common.h>
#include <cebu/simplicial_complex.h>
#include <cebu/change_tracker.h>
#include <cebu/snapshot_manager.h>
#include <nlohmann/json.hpp>
#include <string>
#include <map>
#include <vector>
#include <memory>
#include <functional>

namespace cebu {

/**
 * @brief 版本 ID 类型
 */
using VersionID = size_t;

/**
 * @brief 版本元数据
 */
struct VersionMetadata {
    VersionID id;
    std::string timestamp;
    std::string message;
    std::string author;
    VersionID parent_id;
    std::string branch;
    size_t size;
    size_t compressed_size;
    size_t simplex_count;
    size_t change_count;
    
    VersionMetadata()
        : id(0), parent_id(0), size(0),
          compressed_size(0), simplex_count(0), change_count(0) {}
};

/**
 * @brief 分支信息
 */
struct BranchInfo {
    std::string name;
    VersionID head_id;
    std::string timestamp;
    std::string description;
    
    BranchInfo()
        : head_id(0) {}
};

/**
 * @brief 标签信息
 */
struct TagInfo {
    std::string name;
    VersionID version_id;
    std::string timestamp;
    std::string description;
    
    TagInfo()
        : version_id(0) {}
};

/**
 * @brief 版本控制系统
 * 
 * 提供类似 Git 的版本控制功能，支持提交、切换、分支、合并等操作
 */
class VersionControl {
public:
    /**
     * @brief 构造函数
     * @param filename 版本控制文件名
     */
    explicit VersionControl(const std::string& filename = "versions.ceb");

    /**
     * @brief 析构函数
     */
    ~VersionControl() = default;

    /**
     * @brief 提交版本
     * @param complex 复形对象
     * @param message 提交信息
     * @param author 作者
     * @return 版本 ID
     */
    VersionID commit(
        const SimplicialComplex& complex,
        const std::string& message,
        const std::string& author = "anonymous");

    /**
     * @brief 提交标签复形版本
     */
    template<typename LabelType>
    VersionID commit_labeled(
        const SimplicialComplexLabeled<LabelType>& complex,
        const std::string& message,
        const std::string& author = "anonymous");

    /**
     * @brief 切换版本
     * @param target 目标复形
     * @param version_id 版本 ID
     * @return 是否成功
     */
    bool checkout(SimplicialComplex& target, VersionID version_id);

    /**
     * @brief 查看版本历史
     * @param limit 限制数量（0 表示全部）
     * @return 版本元数据列表
     */
    std::vector<VersionMetadata> log(size_t limit = 0) const;

    /**
     * @brief 获取版本差异
     * @param version1 版本 1 ID
     * @param version2 版本 2 ID
     * @return 变化列表
     */
    std::vector<Change> diff(VersionID version1, VersionID version2) const;

    /**
     * @brief 创建分支
     * @param name 分支名称
     * @param base_id 基础版本 ID（0 表示当前 HEAD）
     * @return 分支名称
     */
    std::string create_branch(const std::string& name, VersionID base_id = 0);

    /**
     * @brief 切换分支
     * @param branch_name 分支名称
     * @return 是否成功
     */
    bool checkout_branch(std::string& branch_name);

    /**
     * @brief 合并分支
     * @param target 目标复形
     * @param branch_name 分支名称
     * @return 是否成功
     */
    bool merge_branch(SimplicialComplex& target, const std::string& branch_name);

    /**
     * @brief 删除分支
     * @param name 分支名称
     * @return 是否成功
     */
    bool delete_branch(const std::string& name);

    /**
     * @brief 列出所有分支
     * @return 分支信息列表
     */
    std::vector<BranchInfo> list_branches() const;

    /**
     * @brief 打标签
     * @param name 标签名称
     * @param version_id 版本 ID
     * @param description 描述
     * @return 标签名称
     */
    std::string create_tag(
        const std::string& name,
        VersionID version_id,
        const std::string& description = "");

    /**
     * @brief 删除标签
     * @param name 标签名称
     * @return 是否成功
     */
    bool delete_tag(const std::string& name);

    /**
     * @brief 列出所有标签
     * @return 标签信息列表
     */
    std::vector<TagInfo> list_tags() const;

    /**
     * @brief 回退版本
     * @param target 目标复形
     * @param version_id 目标版本 ID
     * @param create_commit 是否创建新提交
     * @return 是否成功
     */
    bool revert(
        SimplicialComplex& target,
        VersionID version_id,
        bool create_commit = true);

    /**
     * @brief 获取当前 HEAD 版本 ID
     * @return 版本 ID
     */
    VersionID get_head() const;

    /**
     * @brief 获取当前分支
     * @return 分支名称
     */
    std::string get_current_branch() const;

    /**
     * @brief 获取版本
     * @param version_id 版本 ID
     * @return 复形对象
     */
    SimplicialComplex get_version(VersionID version_id) const;

    /**
     * @brief 保存版本控制数据到文件
     * @param filename 文件名
     * @return 是否成功
     */
    bool save_to_file(const std::string& filename = "") const;

    /**
     * @brief 从文件加载版本控制数据
     * @param filename 文件名
     * @return 是否成功
     */
    bool load_from_file(const std::string& filename = "");

    /**
     * @brief 获取版本数量
     * @return 版本数量
     */
    size_t get_version_count() const;

    /**
     * @brief 获取分支数量
     * @return 分支数量
     */
    size_t get_branch_count() const;

    /**
     * @brief 获取标签数量
     * @return 标签数量
     */
    size_t get_tag_count() const;

private:
    std::map<VersionID, VersionMetadata> versions_;
    std::map<std::string, BranchInfo> branches_;
    std::map<std::string, TagInfo> tags_;
    std::map<VersionID, nlohmann::json> version_data_;
    VersionID head_;
    std::string current_branch_;
    VersionID next_version_id_;
    std::string filename_;
    
    /**
     * @brief 生成时间戳
     * @return 时间戳字符串
     */
    std::string generate_timestamp() const;
    
    /**
     * @brief 计算版本间的差异
     * @param version1 版本 1 数据
     * @param version2 版本 2 数据
     * @return 变化列表
     */
    std::vector<Change> compute_diff(
        const nlohmann::json& version1,
        const nlohmann::json& version2) const;
    
    /**
     * @brief 应用差异
     * @param target 目标复形
     * @param base_id 基础版本 ID
     * @param target_id 目标版本 ID
     * @return 是否成功
     */
    bool apply_diff(
        SimplicialComplex& target,
        VersionID base_id,
        VersionID target_id);
};

// Template implementations
template<typename LabelType>
VersionID VersionControl::commit_labeled(
    const SimplicialComplexLabeled<LabelType>& complex,
    const std::string& message,
    const std::string& author) {
    
    // Serialize to JSON
    nlohmann::json j = JsonSerializer::serialize_labeled(complex);
    
    // Create version metadata
    VersionMetadata metadata;
    metadata.id = next_version_id_++;
    metadata.timestamp = generate_timestamp();
    metadata.message = message;
    metadata.author = author;
    metadata.parent_id = head_;
    metadata.branch = current_branch_;
    metadata.size = j.dump().size();
    metadata.simplex_count = complex.simplex_count();
    metadata.change_count = metadata.id - metadata.parent_id; // Simplified
    
    // Store version
    versions_[metadata.id] = metadata;
    version_data_[metadata.id] = j;
    
    // Update HEAD
    head_ = metadata.id;
    
    // Update branch HEAD
    if (branches_.find(current_branch_) != branches_.end()) {
        branches_[current_branch_].head_id = head_;
        branches_[current_branch_].timestamp = metadata.timestamp;
    }
    
    return metadata.id;
}

} // namespace cebu
