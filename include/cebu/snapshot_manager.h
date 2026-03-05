#pragma once

#include <cebu/common.h>
#include <cebu/simplicial_complex.h>
#include <cebu/json_serialization.h>
#include <cebu/compression.h>
#include <string>
#include <map>
#include <memory>
#include <vector>
#include <fstream>

namespace cebu {

/**
 * @brief 快照元数据
 */
struct SnapshotMetadata {
    std::string name;
    std::string timestamp;
    std::string hash;
    size_t size;
    size_t compressed_size;
    size_t simplex_count;
    size_t vertex_count;
    size_t max_dimension;
    
    SnapshotMetadata()
        : size(0), compressed_size(0), simplex_count(0),
          vertex_count(0), max_dimension(0) {}
};

/**
 * @brief 快照数据
 */
struct Snapshot {
    SnapshotMetadata metadata;
    nlohmann::json data;  // JSON representation of the complex
    std::vector<uint8_t> compressed_data;
    bool is_compressed;
    
    Snapshot()
        : is_compressed(false) {}
};

/**
 * @brief 快照管理器
 * 
 * 管理复形的快照，支持创建、恢复、比较等操作
 */
class SnapshotManager {
public:
    /**
     * @brief 构造函数
     * @param filename 快照文件名（可选）
     */
    explicit SnapshotManager(const std::string& filename = "snapshots.ceb");

    /**
     * @brief 析构函数
     */
    ~SnapshotManager() = default;

    /**
     * @brief 创建快照
     * @param complex 复形
     * @param name 快照名称
     * @param compress 是否压缩
     * @return 快照元数据
     */
    SnapshotMetadata create_snapshot(
        const SimplicialComplex& complex,
        const std::string& name,
        bool compress = true);

    /**
     * @brief 创建标签复形快照
     */
    template<typename LabelType>
    SnapshotMetadata create_snapshot_labeled(
        const SimplicialComplexLabeled<LabelType>& complex,
        const std::string& name,
        bool compress = true);

    /**
     * @brief 创建叙事复形快照
     */
    template<typename LabelType>
    SnapshotMetadata create_snapshot_narrative(
        const SimplicialComplexNarrative<LabelType>& complex,
        const std::string& name,
        bool compress = true);

    /**
     * @brief 列出所有快照
     * @return 快照元数据列表
     */
    std::vector<SnapshotMetadata> list_snapshots() const;

    /**
     * @brief 获取快照
     * @param name 快照名称
     * @return 快照数据
     */
    std::shared_ptr<Snapshot> get_snapshot(const std::string& name) const;

    /**
     * @brief 删除快照
     * @param name 快照名称
     * @return 是否成功
     */
    bool delete_snapshot(const std::string& name);

    /**
     * @brief 恢复快照
     * @param target 目标复形
     * @param name 快照名称
     * @return 是否成功
     */
    bool restore_snapshot(
        SimplicialComplex& target,
        const std::string& name) const;

    /**
     * @brief 比较两个快照
     * @param name1 快照 1 名称
     * @param name2 快照 2 名称
     * @return 变化列表（Change 对象）
     */
    std::vector<Change> compare_snapshots(
        const std::string& name1,
        const std::string& name2) const;

    /**
     * @brief 保存快照管理器到文件
     * @param filename 文件名
     * @return 是否成功
     */
    bool save_to_file(const std::string& filename = "") const;

    /**
     * @brief 从文件加载快照管理器
     * @param filename 文件名
     * @return 是否成功
     */
    bool load_from_file(const std::string& filename = "");

    /**
     * @brief 获取快照数量
     * @return 快照数量
     */
    size_t get_snapshot_count() const;

    /**
     * @brief 检查快照是否存在
     * @param name 快照名称
     * @return 是否存在
     */
    bool has_snapshot(const std::string& name) const;

    /**
     * @brief 获取快照总大小（压缩后）
     * @return 总大小（字节）
     */
    size_t get_total_size() const;

    /**
     * @brief 获取快照总大小（未压缩）
     * @return 总大小（字节）
     */
    size_t get_total_uncompressed_size() const;

    /**
     * @brief 清空所有快照
     */
    void clear();

private:
    std::map<std::string, std::shared_ptr<Snapshot>> snapshots_;
    std::string filename_;
    
    /**
     * @brief 计算复形的哈希值
     * @param complex 复形
     * @return 哈希值字符串
     */
    std::string compute_hash(const SimplicialComplex& complex) const;
    
    /**
     * @brief 压缩快照数据
     * @param snapshot 快照
     * @param level 压缩级别
     * @return 是否成功
     */
    bool compress_snapshot(Snapshot& snapshot, int level = 6) const;
    
    /**
     * @brief 解压缩快照数据
     * @param snapshot 快照
     * @return 是否成功
     */
    bool decompress_snapshot(Snapshot& snapshot) const;
    
    /**
     * @brief 创建元数据
     * @param complex 复形
     * @param name 快照名称
     * @return 元数据
     */
    SnapshotMetadata create_metadata(
        const SimplicialComplex& complex,
        const std::string& name) const;
};

// Template implementations
template<typename LabelType>
SnapshotMetadata SnapshotManager::create_snapshot_labeled(
    const SimplicialComplexLabeled<LabelType>& complex,
    const std::string& name,
    bool compress) {
    
    // Create snapshot
    auto snapshot = std::make_shared<Snapshot>();
    
    // Serialize to JSON
    snapshot->data = JsonSerializer::serialize_labeled(complex);
    
    // Create metadata
    snapshot->metadata = create_metadata(complex, name);
    snapshot->metadata.simplex_count = complex.simplex_count();
    snapshot->metadata.vertex_count = complex.vertex_count();
    snapshot->metadata.max_dimension = complex.max_dimension();
    snapshot->metadata.size = snapshot->data.dump().size();
    
    // Compress if requested
    if (compress && Compression::has_zlib_support()) {
        compress_snapshot(*snapshot);
    }
    
    // Store snapshot
    snapshots_[name] = snapshot;
    
    return snapshot->metadata;
}

template<typename LabelType>
SnapshotMetadata SnapshotManager::create_snapshot_narrative(
    const SimplicialComplexNarrative<LabelType>& complex,
    const std::string& name,
    bool compress) {
    
    // Create snapshot
    auto snapshot = std::make_shared<Snapshot>();
    
    // Serialize to JSON
    snapshot->data = JsonSerializer::serialize_narrative(complex);
    
    // Create metadata
    snapshot->metadata = create_metadata(complex, name);
    snapshot->metadata.simplex_count = complex.simplex_count();
    snapshot->metadata.vertex_count = complex.vertex_count();
    snapshot->metadata.max_dimension = complex.max_dimension();
    snapshot->metadata.size = snapshot->data.dump().size();
    
    // Compress if requested
    if (compress && Compression::has_zlib_support()) {
        compress_snapshot(*snapshot);
    }
    
    // Store snapshot
    snapshots_[name] = snapshot;
    
    return snapshot->metadata;
}

} // namespace cebu
