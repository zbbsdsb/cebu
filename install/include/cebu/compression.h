#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace cebu {

/// Compression utilities for serialization data
class Compression {
public:
    /// Compression algorithms
    enum class Algorithm {
        NONE,   ///< No compression
        ZLIB    ///< ZLIB compression
    };
    
    /// Compression levels (0-9, where 0 = no compression, 9 = max compression)
    static constexpr int DEFAULT_LEVEL = 6;
    static constexpr int MAX_LEVEL = 9;
    static constexpr int MIN_LEVEL = 0;
    
    /// Compress data using specified algorithm
    /// @param data Input data to compress
    /// @param algorithm Compression algorithm to use
    /// @param level Compression level (0-9, only used for ZLIB)
    /// @return Compressed data
    /// @throws std::runtime_error if compression fails
    static std::vector<uint8_t> compress(
        const std::vector<uint8_t>& data,
        Algorithm algorithm = Algorithm::NONE,
        int level = DEFAULT_LEVEL);
    
    /// Decompress data
    /// @param compressed Compressed data
    /// @param algorithm Compression algorithm that was used
    /// @return Decompressed data
    /// @throws std::runtime_error if decompression fails
    static std::vector<uint8_t> decompress(
        const std::vector<uint8_t>& compressed,
        Algorithm algorithm = Algorithm::NONE);
    
    /// Compress using ZLIB
    static std::vector<uint8_t> compress_zlib(
        const std::vector<uint8_t>& data,
        int level = DEFAULT_LEVEL);
    
    /// Decompress ZLIB data
    static std::vector<uint8_t> decompress_zlib(
        const std::vector<uint8_t>& compressed);
    
    /// Get compression info for data
    struct CompressionInfo {
        size_t original_size;
        size_t compressed_size;
        double compression_ratio;
        Algorithm algorithm;
        int level;
    };
    
    /// Compress and return both data and info
    static std::pair<std::vector<uint8_t>, CompressionInfo> compress_with_info(
        const std::vector<uint8_t>& data,
        Algorithm algorithm = Algorithm::NONE,
        int level = DEFAULT_LEVEL);
    
    /// Check if data is ZLIB compressed
    /// ZLIB data starts with magic bytes 0x78 followed by a checksum byte
    static bool is_zlib_compressed(const std::vector<uint8_t>& data);
    
    /// Estimate compression ratio for data (without actually compressing)
    /// This is a heuristic based on data entropy
    static double estimate_compression_ratio(const std::vector<uint8_t>& data);
    
    /// Get recommended compression level based on data size
    static int get_recommended_level(size_t data_size);
    
    /// Check if ZLIB support is available (compiled with zlib)
    static bool has_zlib_support();
    
    /// Get ZLIB version string
    static std::string get_zlib_version();

private:
    /// ZLIB magic bytes
    static constexpr uint8_t ZLIB_DEFLATE = 0x78;
};

} // namespace cebu
