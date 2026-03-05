#include "cebu/streaming_io.h"
#include <filesystem>

namespace cebu {

// StreamingLoader implementation
StreamingLoader::StreamingLoader(const std::string& filename)
    : filename_(filename),
      progress_callback_(nullptr),
      total_simplices_(0),
      total_vertices_(0),
      max_dimension_(0),
      file_size_(0),
      is_compressed_(false) {
    
    analyze_file();
}

void StreamingLoader::set_progress_callback(ProgressCallback callback) {
    progress_callback_ = callback;
}

SimplicialComplex StreamingLoader::load() {
    // Read header
    nlohmann::json j = read_header();
    
    if (progress_callback_) {
        progress_callback_(1, 10);
    }
    
    // Deserialize
    SimplicialComplex complex = JsonSerializer::deserialize(j);
    
    if (progress_callback_) {
        progress_callback_(10, 10);
    }
    
    return complex;
}

SimplicialComplex StreamingLoader::load_partial(const std::vector<SimplexID>& simplex_ids) {
    // Read full file first (simplified implementation)
    nlohmann::json j = read_header();
    
    if (progress_callback_) {
        progress_callback_(5, 10);
    }
    
    // Deserialize full complex
    SimplicialComplex complex = JsonSerializer::deserialize(j);
    
    // Create new complex with only specified simplices
    SimplicialComplex result;
    
    // Note: This is a simplified implementation
    // A proper implementation would need to:
    // 1. Find all vertices needed for the specified simplices
    // 2. Add those vertices to the result complex
    // 3. Add only the specified simplices
    
    if (progress_callback_) {
        progress_callback_(10, 10);
    }
    
    return result;
}

size_t StreamingLoader::get_total_simplices() const {
    return total_simplices_;
}

size_t StreamingLoader::get_total_vertices() const {
    return total_vertices_;
}

size_t StreamingLoader::get_max_dimension() const {
    return max_dimension_;
}

size_t StreamingLoader::get_file_size() const {
    return file_size_;
}

bool StreamingLoader::is_compressed() const {
    return is_compressed_;
}

void StreamingLoader::analyze_file() {
    try {
        // Get file size
        file_size_ = std::filesystem::file_size(filename_);
        
        // Check if compressed
        std::ifstream in(filename_, std::ios::binary);
        if (in) {
            std::vector<uint8_t> header(2);
            in.read(reinterpret_cast<char*>(header.data()), 2);
            in.close();
            
            is_compressed_ = Compression::is_zlib_compressed(header);
        }
        
        // Read JSON to get metadata (for uncompressed files)
        if (!is_compressed_) {
            nlohmann::json j = read_header();
            if (j.contains("statistics")) {
                total_simplices_ = j["statistics"]["simplex_count"].get<size_t>();
                total_vertices_ = j["statistics"]["vertex_count"].get<size_t>();
                max_dimension_ = j["statistics"]["max_dimension"].get<size_t>();
            }
        }
    } catch (const std::exception& e) {
        // Keep default values
    }
}

nlohmann::json StreamingLoader::read_header() {
    try {
        std::ifstream in(filename_);
        if (!in) {
            throw std::runtime_error("Failed to open file: " + filename_);
        }
        
        nlohmann::json j;
        in >> j;
        in.close();
        
        return j;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to read file: " + std::string(e.what()));
    }
}

// StreamingWriter implementation
StreamingWriter::StreamingWriter(const std::string& filename)
    : filename_(filename),
      progress_callback_(nullptr),
      compress_(false),
      compression_level_(6) {}

void StreamingWriter::set_progress_callback(ProgressCallback callback) {
    progress_callback_ = callback;
}

bool StreamingWriter::write(const SimplicialComplex& complex) {
    try {
        std::ofstream out(filename_);
        if (!out) {
            return false;
        }
        
        // Serialize to JSON
        nlohmann::json j = JsonSerializer::serialize(complex);
        
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

bool StreamingWriter::write_partial(
    const SimplicialComplex& complex,
    const std::vector<SimplexID>& simplex_ids) {
    
    try {
        // For simplicity, we serialize the full complex but filter
        // In a real implementation, this would be more efficient
        nlohmann::json j = JsonSerializer::serialize(complex);
        
        // Filter simplices
        nlohmann::json filtered_simplices = nlohmann::json::array();
        
        std::unordered_set<SimplexID> id_set(simplex_ids.begin(), simplex_ids.end());
        
        for (const auto& simplex : j["simplices"]) {
            SimplexID id = simplex["id"].get<SimplexID>();
            if (id_set.find(id) != id_set.end()) {
                filtered_simplices.push_back(simplex);
            }
        }
        
        j["simplices"] = filtered_simplices;
        
        // Write to file
        std::ofstream out(filename_);
        if (!out) {
            return false;
        }
        
        out << j.dump(2);
        out.close();
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

void StreamingWriter::set_compression(bool compress, int level) {
    compress_ = compress;
    compression_level_ = level;
}

void StreamingWriter::write_header(std::ofstream& out, const SimplicialComplex& complex) {
    nlohmann::json header;
    header["format"] = "cebu";
    header["version"] = "0.7.2";
    header["simplex_count"] = complex.simplex_count();
    header["vertex_count"] = complex.vertex_count();
    header["max_dimension"] = complex.max_dimension();
    
    out << header.dump(2) << std::endl;
}

void StreamingWriter::write_simplices(
    std::ofstream& out,
    const nlohmann::json& simplices,
    size_t& current,
    size_t total) {
    
    for (const auto& simplex : simplices) {
        out << simplex.dump() << std::endl;
        current++;
        
        if (progress_callback_ && current % 100 == 0) {
            progress_callback_(current, total);
        }
    }
    
    if (progress_callback_) {
        progress_callback_(total, total);
    }
}

// ChunkedStreamingLoader implementation
ChunkedStreamingLoader::ChunkedStreamingLoader(
    const std::string& filename,
    size_t chunk_size)
    : filename_(filename), chunk_size_(chunk_size) {
    
    analyze_chunks();
}

std::vector<ChunkInfo> ChunkedStreamingLoader::get_chunks() const {
    return chunks_;
}

nlohmann::json ChunkedStreamingLoader::load_chunk(size_t chunk_index) {
    if (chunk_index >= chunks_.size()) {
        throw std::runtime_error("Invalid chunk index");
    }
    
    try {
        std::ifstream in(filename_, std::ios::binary);
        if (!in) {
            throw std::runtime_error("Failed to open file");
        }
        
        // Seek to chunk offset
        in.seekg(chunks_[chunk_index].offset);
        
        // Read chunk data
        std::vector<char> buffer(chunks_[chunk_index].size);
        in.read(buffer.data(), chunks_[chunk_index].size);
        in.close();
        
        // Parse JSON
        std::string json_str(buffer.begin(), buffer.end());
        return nlohmann::json::parse(json_str);
        
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to load chunk: " + std::string(e.what()));
    }
}

nlohmann::json ChunkedStreamingLoader::load_all_chunks(ProgressCallback progress_callback) {
    nlohmann::json result;
    
    try {
        std::ifstream in(filename_);
        if (!in) {
            throw std::runtime_error("Failed to open file");
        }
        
        // Read entire file (simplified)
        nlohmann::json j;
        in >> j;
        in.close();
        
        if (progress_callback) {
            progress_callback_(1, 1);
        }
        
        return j;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to load chunks: " + std::string(e.what()));
    }
}

void ChunkedStreamingLoader::analyze_chunks() {
    try {
        // Get file size
        size_t file_size = std::filesystem::file_size(filename_);
        
        // Calculate number of chunks
        size_t num_chunks = (file_size + chunk_size_ - 1) / chunk_size_;
        
        // Create chunk info
        chunks_.clear();
        for (size_t i = 0; i < num_chunks; ++i) {
            ChunkInfo info;
            info.offset = i * chunk_size_;
            info.size = std::min(chunk_size_, file_size - info.offset);
            info.simplex_count = 0; // Would need proper parsing
            info.checksum = ""; // Would need proper calculation
            
            chunks_.push_back(info);
        }
    } catch (const std::exception& e) {
        // Keep empty chunks
    }
}

} // namespace cebu
