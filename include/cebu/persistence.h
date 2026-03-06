#ifndef CEBU_PERSISTENCE_H
#define CEBU_PERSISTENCE_H

#include "cebu/serialization.h"
#include "cebu/simplicial_complex.h"
#include "cebu/simplicial_complex_labeled.h"
#include "cebu/simplicial_complex_narrative.h"
#include "cebu/refinement.h"
#include "cebu/simplicial_complex_non_hausdorff.h"
#include "cebu/compression.h"
#include <string>
#include <vector>
#include <fstream>
#include <memory>

namespace cebu {

/**
 * @brief File format for persistence
 */
enum class FileFormat {
    AUTO,      /// Auto-detect from extension
    BINARY,    /// Binary format (.bin, .ceb)
    JSON       /// JSON format (.json)
};

/**
 * @brief Options for persistence operations
 */
struct PersistenceOptions {
    FileFormat format = FileFormat::AUTO;
    Compression::Algorithm compression = Compression::Algorithm::NONE;
    int compression_level = 6;  // 0-9, only used for ZLIB
    bool validate_after_load = true;
    bool include_metadata = true;
    bool save_command_history = false;
    bool save_equivalence_classes = true;
    std::string custom_metadata;
};

/**
 * @brief Metadata for saved files
 */
struct FileMetadata {
    std::string cebu_version;
    std::string format_version;
    uint64_t simplex_count;
    uint64_t vertex_count;
    uint64_t file_size;
    std::string created_at;
    std::string modified_at;
};

/**
 * @brief Result of a load operation
 */
template<typename ComplexType>
struct LoadResult {
    ComplexType complex;
    FileMetadata metadata;
    std::string error_message;
    bool success = false;
};

/**
 * @brief Convenience class for file I/O operations
 *
 * Provides high-level interface for saving/loading simplicial complexes
 * to/from files with automatic format detection and optional compression.
 */
class Persistence {
public:
    /**
     * @brief Save a basic simplicial complex to file
     */
    static bool save(const SimplicialComplex& complex,
                    const std::string& filename,
                    const PersistenceOptions& options = {});

    /**
     * @brief Save a labeled simplicial complex to file
     */
    template<typename LabelType>
    static bool save_labeled(const SimplicialComplexLabeled<LabelType>& complex,
                           const std::string& filename,
                           const PersistenceOptions& options = {});

    /**
     * @brief Save a narrative complex to file
     */
    template<typename LabelType>
    static bool save_narrative(const SimplicialComplexNarrative<LabelType>& complex,
                             const std::string& filename,
                             const PersistenceOptions& options = {});

    /**
     * @brief Save a refinement complex to file
     */
    template<typename LabelType>
    static bool save_refinement(const SimplicialComplexRefinement<LabelType>& complex,
                              const std::string& filename,
                              const PersistenceOptions& options = {});

    /**
     * @brief Load a basic simplicial complex from file
     */
    static LoadResult<SimplicialComplex> load(
        const std::string& filename,
        const PersistenceOptions& options = {});

    /**
     * @brief Load a labeled simplicial complex from file
     */
    template<typename LabelType>
    static LoadResult<SimplicialComplexLabeled<LabelType>> load_labeled(
        const std::string& filename,
        const PersistenceOptions& options = {});

    /**
     * @brief Load a narrative complex from file
     */
    template<typename LabelType>
    static LoadResult<SimplicialComplexNarrative<LabelType>> load_narrative(
        const std::string& filename,
        const PersistenceOptions& options = {});

    /**
     * @brief Load a refinement complex from file
     */
    template<typename LabelType>
    static LoadResult<SimplicialComplexRefinement<LabelType>> load_refinement(
        const std::string& filename,
        const PersistenceOptions& options = {});

    /**
     * @brief Detect file format from filename
     */
    static FileFormat detect_format(const std::string& filename);

    /**
     * @brief Get metadata from a file without loading the full complex
     */
    static FileMetadata get_metadata(const std::string& filename);

    /**
     * @brief Validate a file's integrity
     */
    static bool validate_file(const std::string& filename);

private:
    /**
     * @brief Write binary data to file
     */
    static bool write_binary_file(const std::string& filename,
                                 const std::vector<uint8_t>& data,
                                 Compression compression);

    /**
     * @brief Read binary data from file
     */
    static std::vector<uint8_t> read_binary_file(const std::string& filename);

    /**
     * @brief Write JSON data to file
     */
    static bool write_json_file(const std::string& filename,
                               const std::string& json);

    /**
     * @brief Read JSON data from file
     */
    static std::string read_json_file(const std::string& filename);

    /**
     * @brief Compress data using ZLIB
     */
    static std::vector<uint8_t> compress_zlib(const std::vector<uint8_t>& data);

    /**
     * @brief Decompress data using ZLIB
     */
    static std::vector<uint8_t> decompress_zlib(const std::vector<uint8_t>& data);

    /**
     * @brief Create file metadata
     */
    template<typename ComplexType>
    static FileMetadata create_metadata(const ComplexType& complex,
                                       uint64_t file_size);
};

// ============================================================================
// Template implementations
// ============================================================================

template<typename LabelType>
bool Persistence::save_labeled(const SimplicialComplexLabeled<LabelType>& complex,
                               const std::string& filename,
                               const PersistenceOptions& options) {
    FileFormat format = options.format;
    if (format == FileFormat::AUTO) {
        format = detect_format(filename);
    }

    if (format == FileFormat::BINARY) {
        auto data = BinarySerializer::serialize_labeled(complex);

        // Optionally compress
        std::vector<uint8_t> final_data = data;
        if (options.compression == Compression::ZLIB) {
            final_data = compress_zlib(data);
        }

        return write_binary_file(filename, final_data, Compression::NONE);
    } else if (format == FileFormat::JSON) {
        // Note: JsonSerializer doesn't have a generic serialize_labeled method
        // This would need to be implemented
        std::string json = JsonSerializer::serialize_labeled_double(complex);
        return write_json_file(filename, json);
    }

    return false;
}

template<typename LabelType>
bool Persistence::save_narrative(const SimplicialComplexNarrative<LabelType>& complex,
                                const std::string& filename,
                                const PersistenceOptions& options) {
    FileFormat format = options.format;
    if (format == FileFormat::AUTO) {
        format = detect_format(filename);
    }

    if (format == FileFormat::BINARY) {
        auto data = BinarySerializer::serialize_narrative(complex);

        std::vector<uint8_t> final_data = data;
        if (options.compression == Compression::ZLIB) {
            final_data = compress_zlib(data);
        }

        return write_binary_file(filename, final_data, Compression::NONE);
    } else if (format == FileFormat::JSON) {
        std::string json = JsonSerializer::serialize_narrative(complex);
        return write_json_file(filename, json);
    }

    return false;
}

template<typename LabelType>
bool Persistence::save_refinement(const SimplicialComplexRefinement<LabelType>& complex,
                                 const std::string& filename,
                                 const PersistenceOptions& options) {
    FileFormat format = options.format;
    if (format == FileFormat::AUTO) {
        format = detect_format(filename);
    }

    if (format == FileFormat::BINARY) {
        auto data = BinarySerializer::serialize_refinement(complex);

        std::vector<uint8_t> final_data = data;
        if (options.compression == Compression::ZLIB) {
            final_data = compress_zlib(data);
        }

        return write_binary_file(filename, final_data, Compression::NONE);
    }

    return false;
}

template<typename LabelType>
LoadResult<SimplicialComplexLabeled<LabelType>> Persistence::load_labeled(
    const std::string& filename,
    const PersistenceOptions& options) {

    LoadResult<SimplicialComplexLabeled<LabelType>> result;

    FileFormat format = options.format;
    if (format == FileFormat::AUTO) {
        format = detect_format(filename);
    }

    try {
        if (format == FileFormat::BINARY) {
            auto data = read_binary_file(filename);

            // Optionally decompress
            std::vector<uint8_t> final_data = data;
            // Note: We'd need to detect compression automatically

            result.complex = BinarySerializer::deserialize_labeled<LabelType>(final_data);
            result.success = true;
            result.metadata = create_metadata(result.complex, data.size());
        } else if (format == FileFormat::JSON) {
            // Note: JsonSerializer doesn't have deserialize
            result.error_message = "JSON deserialization not implemented";
            result.success = false;
        }
    } catch (const std::exception& e) {
        result.error_message = e.what();
        result.success = false;
    }

    return result;
}

template<typename LabelType>
LoadResult<SimplicialComplexNarrative<LabelType>> Persistence::load_narrative(
    const std::string& filename,
    const PersistenceOptions& options) {

    LoadResult<SimplicialComplexNarrative<LabelType>> result;

    FileFormat format = options.format;
    if (format == FileFormat::AUTO) {
        format = detect_format(filename);
    }

    try {
        if (format == FileFormat::BINARY) {
            auto data = read_binary_file(filename);
            result.complex = BinarySerializer::deserialize_narrative<LabelType>(data);
            result.success = true;
            result.metadata = create_metadata(result.complex, data.size());
        }
    } catch (const std::exception& e) {
        result.error_message = e.what();
        result.success = false;
    }

    return result;
}

template<typename LabelType>
LoadResult<SimplicialComplexRefinement<LabelType>> Persistence::load_refinement(
    const std::string& filename,
    const PersistenceOptions& options) {

    LoadResult<SimplicialComplexRefinement<LabelType>> result;

    FileFormat format = options.format;
    if (format == FileFormat::AUTO) {
        format = detect_format(filename);
    }

    try {
        if (format == FileFormat::BINARY) {
            auto data = read_binary_file(filename);
            result.complex = BinarySerializer::deserialize_refinement<LabelType>(data);
            result.success = true;
            result.metadata = create_metadata(result.complex, data.size());
        }
    } catch (const std::exception& e) {
        result.error_message = e.what();
        result.success = false;
    }

    return result;
}

template<typename ComplexType>
FileMetadata Persistence::create_metadata(const ComplexType& complex,
                                          uint64_t file_size) {
    FileMetadata metadata;
    metadata.cebu_version = "0.7.0";
    metadata.format_version = "1.0";
    metadata.simplex_count = complex.simplex_count();
    metadata.vertex_count = complex.vertex_count();
    metadata.file_size = file_size;
    metadata.created_at = "unknown";
    metadata.modified_at = "unknown";

    return metadata;
}

} // namespace cebu

#endif // CEBU_PERSISTENCE_H
