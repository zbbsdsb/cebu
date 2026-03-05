#pragma once

#include <cebu/common.h>
#include <cebu/simplicial_complex.h>
#include <cebu/json_serialization.h>
#include <cebu/compression.h>
#include <string>
#include <vector>
#include <functional>
#include <fstream>

namespace cebu {

/**
 * @brief 进度回调函数类型
 */
using ProgressCallback = std::function<void(size_t current, size_t total)>;

/**
 * @brief 流式加载器
 * 
 * 支持大型文件的流式加载，减少内存使用
 */
class StreamingLoader {
public:
    /**
     * @brief 构造函数
     * @param filename 文件名
     */
    explicit StreamingLoader(const std::string& filename);

    /**
     * @brief 析构函数
     */
    ~StreamingLoader() = default;

    /**
     * @brief 设置进度回调
     * @param callback 回调函数
     */
    void set_progress_callback(ProgressCallback callback);

    /**
     * @brief 加载整个复形
     * @return 复形对象
     */
    SimplicialComplex load();

    /**
     * @brief 加载标签复形
     */
    template<typename LabelType>
    SimplicialComplexLabeled<LabelType> load_labeled();

    /**
     * @brief 部分加载（只加载指定的单纯形）
     * @param simplex_ids 单纯形 ID 列表
     * @return 复形对象（只包含指定的单纯形）
     */
    SimplicialComplex load_partial(const std::vector<SimplexID>& simplex_ids);

    /**
     * @brief 获取总单纯形数量（不加载完整数据）
     * @return 单纯形数量
     */
    size_t get_total_simplices() const;

    /**
     * @brief 获取总顶点数量
     * @return 顶点数量
     */
    size_t get_total_vertices() const;

    /**
     * @brief 获取最大维度
     * @return 最大维度
     */
    size_t get_max_dimension() const;

    /**
     * @brief 获取文件大小
     * @return 文件大小（字节）
     */
    size_t get_file_size() const;

    /**
     * @brief 检查是否为压缩文件
     * @return 是否压缩
     */
    bool is_compressed() const;

private:
    std::string filename_;
    ProgressCallback progress_callback_;
    size_t total_simplices_;
    size_t total_vertices_;
    size_t max_dimension_;
    size_t file_size_;
    bool is_compressed_;
    
    /**
     * @brief 分析文件元数据
     */
    void analyze_file();
    
    /**
     * @brief 读取文件头
     * @return JSON 对象
     */
    nlohmann::json read_header();
};

/**
 * @brief 流式写入器
 * 
 * 支持大型文件的流式写入，减少内存使用
 */
class StreamingWriter {
public:
    /**
     * @brief 构造函数
     * @param filename 文件名
     */
    explicit StreamingWriter(const std::string& filename);

    /**
     * @brief 析构函数
     */
    ~StreamingWriter() = default;

    /**
     * @brief 设置进度回调
     * @param callback 回调函数
     */
    void set_progress_callback(ProgressCallback callback);

    /**
     * @brief 写入整个复形
     * @param complex 复形对象
     * @return 是否成功
     */
    bool write(const SimplicialComplex& complex);

    /**
     * @brief 写入标签复形
     */
    template<typename LabelType>
    bool write_labeled(const SimplicialComplexLabeled<LabelType>& complex);

    /**
     * @brief 部分写入（只写入指定的单纯形）
     * @param complex 复形对象
     * @param simplex_ids 单纯形 ID 列表
     * @return 是否成功
     */
    bool write_partial(
        const SimplicialComplex& complex,
        const std::vector<SimplexID>& simplex_ids);

    /**
     * @brief 设置压缩
     * @param compress 是否压缩
     * @param level 压缩级别
     */
    void set_compression(bool compress, int level = 6);

private:
    std::string filename_;
    ProgressCallback progress_callback_;
    bool compress_;
    int compression_level_;
    
    /**
     * @brief 写入文件头
     * @param out 输出流
     * @param complex 复形对象
     */
    void write_header(std::ofstream& out, const SimplicialComplex& complex);
    
    /**
     * @brief 写入单纯形数据
     * @param out 输出流
     * @param simplices 单纯形列表
     * @param current 当前进度
     * @param total 总数量
     */
    void write_simplices(
        std::ofstream& out,
        const nlohmann::json& simplices,
        size_t& current,
        size_t total);
};

/**
 * @brief 分块信息
 */
struct ChunkInfo {
    size_t offset;
    size_t size;
    size_t simplex_count;
    std::string checksum;
};

/**
 * @brief 分块流式加载器
 * 
 * 支持分块加载和并行处理
 */
class ChunkedStreamingLoader {
public:
    /**
     * @brief 构造函数
     * @param filename 文件名
     * @param chunk_size 块大小（字节）
     */
    ChunkedStreamingLoader(const std::string& filename, size_t chunk_size = 1024 * 1024);

    /**
     * @brief 析构函数
     */
    ~ChunkedStreamingLoader() = default;

    /**
     * @brief 获取所有分块信息
     * @return 分块信息列表
     */
    std::vector<ChunkInfo> get_chunks() const;

    /**
     * @brief 加载指定分块
     * @param chunk_index 分块索引
     * @return JSON 对象
     */
    nlohmann::json load_chunk(size_t chunk_index);

    /**
     * @brief 加载所有分块
     * @param progress_callback 进度回调
     * @return 完整 JSON 对象
     */
    nlohmann::json load_all_chunks(ProgressCallback progress_callback = nullptr);

private:
    std::string filename_;
    size_t chunk_size_;
    std::vector<ChunkInfo> chunks_;
    
    /**
     * @brief 分析文件并生成分块信息
     */
    void analyze_chunks();
};

// Template implementations
template<typename LabelType>
SimplicialComplexLabeled<LabelType> StreamingLoader::load_labeled() {
    // Read header
    nlohmann::json j = read_header();
    
    if (progress_callback_) {
        progress_callback_(1, 10);
    }
    
    // Deserialize
    SimplicialComplexLabeled<LabelType> complex = JsonSerializer::deserialize_labeled<LabelType>(j);
    
    if (progress_callback_) {
        progress_callback_(10, 10);
    }
    
    return complex;
}

template<typename LabelType>
bool StreamingWriter::write_labeled(const SimplicialComplexLabeled<LabelType>& complex) {
    try {
        std::ofstream out(filename_);
        if (!out) {
            return false;
        }
        
        // Serialize to JSON
        nlohmann::json j = JsonSerializer::serialize_labeled(complex);
        
        // Write header
        write_header(out, complex);
        
        // Write simplices
        size_t current = 0;
        size_t total = j["statistics"]["simplex_count"].get<size_t>();
        write_simplices(out, j["simplices"], current, total);
        
        out.close();
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

} // namespace cebu
