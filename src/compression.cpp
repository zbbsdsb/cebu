#include "cebu/compression.h"
#include <stdexcept>
#include <cstring>
#include <algorithm>
#include <cmath>

// Check if zlib is available
#ifdef ZLIB_FOUND
#include <zlib.h>
#else
// Stubs for when zlib is not available
#define Z_NO_COMPRESSION 0
#define Z_BEST_SPEED 1
#define Z_BEST_COMPRESSION 9
#define Z_DEFAULT_COMPRESSION 6
#define Z_OK 0
#define Z_MEM_ERROR -4
#define Z_BUF_ERROR -5
#define Z_DATA_ERROR -3
#endif

namespace cebu {

// ============================================================================
// Public API
// ============================================================================

std::vector<uint8_t> Compression::compress(
    const std::vector<uint8_t>& data,
    Algorithm algorithm,
    int level) {
    
    if (data.empty()) {
        return data;
    }
    
    switch (algorithm) {
        case Algorithm::NONE:
            return data;
            
        case Algorithm::ZLIB:
            return compress_zlib(data, level);
            
        default:
            throw std::runtime_error("Unsupported compression algorithm");
    }
}

std::vector<uint8_t> Compression::decompress(
    const std::vector<uint8_t>& compressed,
    Algorithm algorithm) {
    
    if (compressed.empty()) {
        return compressed;
    }
    
    switch (algorithm) {
        case Algorithm::NONE:
            return compressed;
            
        case Algorithm::ZLIB:
            return decompress_zlib(compressed);
            
        default:
            throw std::runtime_error("Unsupported compression algorithm");
    }
}

// ============================================================================
// ZLIB Compression
// ============================================================================

std::vector<uint8_t> Compression::compress_zlib(
    const std::vector<uint8_t>& data,
    int level) {
    
#ifdef ZLIB_FOUND
    if (data.empty()) {
        return data;
    }
    
    // Validate compression level
    level = std::clamp(level, MIN_LEVEL, MAX_LEVEL);
    
    // Calculate bound for compressed data
    uLongf compressed_size = compressBound(static_cast<uLong>(data.size()));
    std::vector<uint8_t> compressed(compressed_size);
    
    // Perform compression
    int result = compress2(
        compressed.data(),
        &compressed_size,
        data.data(),
        static_cast<uLong>(data.size()),
        level);
    
    if (result != Z_OK) {
        std::string error_msg;
        switch (result) {
            case Z_MEM_ERROR:
                error_msg = "Not enough memory";
                break;
            case Z_BUF_ERROR:
                error_msg = "Output buffer too small";
                break;
            case Z_DATA_ERROR:
                error_msg = "Input data corrupted";
                break;
            default:
                error_msg = "Unknown error (code: " + std::to_string(result) + ")";
                break;
        }
        throw std::runtime_error("ZLIB compression failed: " + error_msg);
    }
    
    // Resize to actual compressed size
    compressed.resize(compressed_size);
    return compressed;
    
#else
    // ZLIB not available - return original data with a warning
    // In production, you might want to log a warning here
    return data;
#endif
}

std::vector<uint8_t> Compression::decompress_zlib(
    const std::vector<uint8_t>& compressed) {
    
#ifdef ZLIB_FOUND
    if (compressed.empty()) {
        return compressed;
    }
    
    // Try to determine uncompressed size
    // For ZLIB, we need to guess the size and expand if needed
    uLongf uncompressed_size = compressed.size() * 4;  // Start with 4x estimate
    std::vector<uint8_t> uncompressed(uncompressed_size);
    
    // Try decompression with initial size
    int result;
    while (true) {
        result = uncompress(
            uncompressed.data(),
            &uncompressed_size,
            compressed.data(),
            static_cast<uLong>(compressed.size()));
        
        if (result == Z_OK) {
            break;
        } else if (result == Z_BUF_ERROR) {
            // Buffer too small, try larger
            uncompressed_size *= 2;
            uncompressed.resize(uncompressed_size);
        } else {
            std::string error_msg;
            switch (result) {
                case Z_MEM_ERROR:
                    error_msg = "Not enough memory";
                    break;
                case Z_DATA_ERROR:
                    error_msg = "Compressed data corrupted";
                    break;
                default:
                    error_msg = "Unknown error (code: " + std::to_string(result) + ")";
                    break;
            }
            throw std::runtime_error("ZLIB decompression failed: " + error_msg);
        }
    }
    
    // Resize to actual uncompressed size
    uncompressed.resize(uncompressed_size);
    return uncompressed;
    
#else
    // ZLIB not available - return original data
    return compressed;
#endif
}

// ============================================================================
// Compression with Info
// ============================================================================

std::pair<std::vector<uint8_t>, Compression::CompressionInfo> 
Compression::compress_with_info(
    const std::vector<uint8_t>& data,
    Algorithm algorithm,
    int level) {
    
    CompressionInfo info;
    info.original_size = data.size();
    info.algorithm = algorithm;
    info.level = (algorithm == Algorithm::ZLIB) ? level : 0;
    
    if (data.empty()) {
        return {data, info};
    }
    
    auto compressed = compress(data, algorithm, level);
    info.compressed_size = compressed.size();
    info.compression_ratio = static_cast<double>(compressed.size()) / 
                              static_cast<double>(data.size());
    
    return {compressed, info};
}

// ============================================================================
// Utility Functions
// ============================================================================

bool Compression::is_zlib_compressed(const std::vector<uint8_t>& data) {
    if (data.size() < 2) {
        return false;
    }
    
    // ZLIB data starts with 0x78 followed by a checksum byte
    // The checksum byte can be: 0x01, 0x5e, 0x9c, 0xda
    uint8_t first = data[0];
    if (first != ZLIB_DEFLATE) {
        return false;
    }
    
    uint8_t second = data[1];
    return (second == 0x01 || second == 0x5e || second == 0x9c || second == 0xda);
}

double Compression::estimate_compression_ratio(const std::vector<uint8_t>& data) {
    if (data.empty()) {
        return 1.0;
    }
    
    // Calculate entropy as a heuristic for compressibility
    const size_t buffer_size = 256;
    std::array<int, buffer_size> freq = {};
    
    // Count byte frequencies
    for (uint8_t byte : data) {
        freq[byte]++;
    }
    
    // Calculate entropy
    double entropy = 0.0;
    for (int count : freq) {
        if (count > 0) {
            double p = static_cast<double>(count) / data.size();
            entropy -= p * std::log2(p);
        }
    }
    
    // Normalize entropy to [0, 1] where 0 = highly compressible, 1 = incompressible
    double normalized_entropy = entropy / 8.0;
    
    // Estimate compression ratio: lower entropy = better compression
    // Use a sigmoid function to map entropy to ratio
    double ratio = 0.1 + 0.9 * (1.0 / (1.0 + std::exp(-10.0 * (normalized_entropy - 0.5))));
    
    return ratio;
}

int Compression::get_recommended_level(size_t data_size) {
    // For small files, use faster compression
    if (data_size < 1024) {          // < 1 KB
        return 1;
    } else if (data_size < 10240) {  // < 10 KB
        return 3;
    } else if (data_size < 102400) { // < 100 KB
        return 5;
    } else if (data_size < 1048576) { // < 1 MB
        return 6;
    } else {                         // >= 1 MB
        return 7;  // Use higher compression for large files
    }
}

bool Compression::has_zlib_support() {
#ifdef ZLIB_FOUND
    return true;
#else
    return false;
#endif
}

std::string Compression::get_zlib_version() {
#ifdef ZLIB_FOUND
    return ZLIB_VERSION;
#else
    return "zlib not available";
#endif
}

} // namespace cebu
