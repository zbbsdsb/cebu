#include "cebu/persistence.h"
#include "cebu/compression.h"
#include "cebu/json_serialization.h"
#include <algorithm>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>

namespace cebu {

// ============================================================================
// Static method implementations
// ============================================================================

bool Persistence::save(const SimplicialComplex& complex,
                     const std::string& filename,
                     const PersistenceOptions& options) {

    FileFormat format = options.format;
    if (format == FileFormat::AUTO) {
        format = detect_format(filename);
    }

    if (format == FileFormat::BINARY) {
        // Binary serialization not yet implemented, fall back to JSON
        auto json = JsonSerializer::serialize(complex);
        std::string json_str = JsonSerializer::pretty_print(json, 2);

        std::vector<uint8_t> data(json_str.begin(), json_str.end());
        std::vector<uint8_t> final_data = data;
        if (options.compression == Compression::Algorithm::ZLIB) {
            final_data = Compression::compress(
                data, Compression::Algorithm::ZLIB, options.compression_level);
        }

        return write_binary_file(filename, final_data);
    } else if (format == FileFormat::JSON) {
        auto json = JsonSerializer::serialize(complex);
        std::string json_str = JsonSerializer::pretty_print(json, 2);
        return write_json_file(filename, json_str);
    }

    return false;
}

LoadResult<SimplicialComplex> Persistence::load(
    const std::string& filename,
    const PersistenceOptions& options) {

    LoadResult<SimplicialComplex> result;

    FileFormat format = options.format;
    if (format == FileFormat::AUTO) {
        format = detect_format(filename);
    }

    try {
        if (format == FileFormat::BINARY) {
            auto data = read_binary_file(filename);

            // Check if compressed and decompress if needed
            std::vector<uint8_t> decompressed_data = data;
            if (Compression::is_zlib_compressed(data)) {
                decompressed_data = Compression::decompress_zlib(data);
            }

            // Binary format currently stores JSON as text
            std::string json_str(decompressed_data.begin(), decompressed_data.end());
            result.complex = JsonSerializer::deserialize(
                nlohmann::json::parse(json_str));
            result.success = true;
            result.metadata = create_metadata(result.complex, data.size());

            if (options.validate_after_load) {
                // Basic validation
                if (result.complex.vertex_count() > 1000000) {
                    result.success = false;
                    result.error_message = "Suspiciously large vertex count";
                }
            }
        } else if (format == FileFormat::JSON) {
            result.complex = JsonSerializer::deserialize(
                nlohmann::json::parse(read_json_file(filename)));
            result.success = true;
            result.metadata = create_metadata(result.complex, 0);
        }
    } catch (const std::exception& e) {
        result.error_message = e.what();
        result.success = false;
    }

    return result;
}

FileFormat Persistence::detect_format(const std::string& filename) {
    // Check file extension
    size_t dot_pos = filename.find_last_of('.');
    if (dot_pos != std::string::npos) {
        std::string ext = filename.substr(dot_pos);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        if (ext == ".json") {
            return FileFormat::JSON;
        } else if (ext == ".bin" || ext == ".ceb") {
            return FileFormat::BINARY;
        }
    }

    // Default to binary
    return FileFormat::BINARY;
}

FileMetadata Persistence::get_metadata(const std::string& filename) {
    FileMetadata metadata;

    try {
        auto data = read_binary_file(filename);

        // Read header
        if (data.size() < 12) {
            return metadata;
        }

        // Check magic number
        uint32_t magic = (static_cast<uint32_t>(data[0]) |
                         (static_cast<uint32_t>(data[1]) << 8) |
                         (static_cast<uint32_t>(data[2]) << 16) |
                         (static_cast<uint32_t>(data[3]) << 24));

        if (magic != 0x43454255) { // "CEBU"
            return metadata;
        }

        uint32_t version = (static_cast<uint32_t>(data[4]) |
                           (static_cast<uint32_t>(data[5]) << 8) |
                           (static_cast<uint32_t>(data[6]) << 16) |
                           (static_cast<uint32_t>(data[7]) << 24));

        metadata.cebu_version = "0.7.0";
        metadata.format_version = std::to_string(version);
        metadata.file_size = data.size();

        // Try to extract simplex count from SIMPLICES section
        // This is simplified; real implementation would parse sections properly
        for (size_t i = 8; i < data.size() - 8; ++i) {
            if (data[i] == 'S' && data[i+1] == 'I' &&
                data[i+2] == 'M' && data[i+3] == 'P') {
                // Found SIMP section, read count (at i+8)
                if (i + 16 < data.size()) {
                    metadata.simplex_count =
                        (static_cast<uint64_t>(data[i+8]) |
                         (static_cast<uint64_t>(data[i+9]) << 8) |
                         (static_cast<uint64_t>(data[i+10]) << 16) |
                         (static_cast<uint64_t>(data[i+11]) << 24) |
                         (static_cast<uint64_t>(data[i+12]) << 32) |
                         (static_cast<uint64_t>(data[i+13]) << 40) |
                         (static_cast<uint64_t>(data[i+14]) << 48) |
                         (static_cast<uint64_t>(data[i+15]) << 56));
                    break;
                }
            }
        }

    } catch (const std::exception&) {
        // Return default metadata
    }

    return metadata;
}

bool Persistence::validate_file(const std::string& filename) {
    try {
        auto data = read_binary_file(filename);

        // Check minimum size
        if (data.size() < 12) {
            return false;
        }

        // Check magic number
        uint32_t magic = (static_cast<uint32_t>(data[0]) |
                         (static_cast<uint32_t>(data[1]) << 8) |
                         (static_cast<uint32_t>(data[2]) << 16) |
                         (static_cast<uint32_t>(data[3]) << 24));

        if (magic != 0x43454255) {
            return false;
        }

        // Check version
        uint32_t version = (static_cast<uint32_t>(data[4]) |
                           (static_cast<uint32_t>(data[5]) << 8) |
                           (static_cast<uint32_t>(data[6]) << 16) |
                           (static_cast<uint32_t>(data[7]) << 24));

        if (version < 1 || version > 10) {
            return false;
        }

        // Try to deserialize
        auto result = Persistence::load(filename);
        return result.success;

    } catch (const std::exception&) {
        return false;
    }
}

bool Persistence::write_binary_file(const std::string& filename,
                                   const std::vector<uint8_t>& data,
                                   Compression compression) {
    std::ofstream out(filename, std::ios::binary);
    if (!out) {
        return false;
    }

    out.write(reinterpret_cast<const char*>(data.data()), data.size());
    out.close();

    return out.good();
}

std::vector<uint8_t> Persistence::read_binary_file(const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    in.seekg(0, std::ios::end);
    size_t size = in.tellg();
    in.seekg(0, std::ios::beg);

    std::vector<uint8_t> data(size);
    in.read(reinterpret_cast<char*>(data.data()), size);
    in.close();

    return data;
}

bool Persistence::write_json_file(const std::string& filename,
                                const std::string& json) {
    std::ofstream out(filename);
    if (!out) {
        return false;
    }

    out << json;
    out.close();

    return out.good();
}

std::string Persistence::read_json_file(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    std::stringstream buffer;
    buffer << in.rdbuf();
    in.close();

    return buffer.str();
}

std::vector<uint8_t> Persistence::compress_zlib(const std::vector<uint8_t>& data) {
    // Placeholder implementation
    // In production, this would use zlib or similar library
    // For now, just return the data uncompressed
    return data;
}

std::vector<uint8_t> Persistence::decompress_zlib(const std::vector<uint8_t>& data) {
    // Placeholder implementation
    // In production, this would use zlib or similar library
    return data;
}

} // namespace cebu
