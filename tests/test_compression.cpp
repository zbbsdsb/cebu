#include "cebu/compression.h"
#include "cebu/serialization.h"
#include "cebu/simplicial_complex.h"
#include <iostream>
#include <cassert>
#include <fstream>
#include <chrono>

using namespace cebu;

void test_no_compression() {
    std::cout << "Testing no compression...\n";
    
    std::vector<uint8_t> data = {1, 2, 3, 4, 5};
    
    auto compressed = Compression::compress(
        data, Compression::Algorithm::NONE);
    
    assert(compressed == data);
    
    auto decompressed = Compression::decompress(
        compressed, Compression::Algorithm::NONE);
    
    assert(decompressed == data);
    
    std::cout << "  ✓ No compression test passed\n\n";
}

void test_zlib_compression() {
    std::cout << "Testing ZLIB compression...\n";
    
    // Check if ZLIB is available
    if (!Compression::has_zlib_support()) {
        std::cout << "  ⚠ ZLIB not available, skipping...\n\n";
        return;
    }
    
    // Create test data with some repetitive pattern
    std::vector<uint8_t> data;
    for (int i = 0; i < 1000; ++i) {
        data.push_back(i % 256);
    }
    
    // Test different compression levels
    for (int level = 1; level <= 9; ++level) {
        auto compressed = Compression::compress_zlib(data, level);
        auto decompressed = Compression::decompress_zlib(compressed);
        
        assert(decompressed == data);
        
        double ratio = static_cast<double>(compressed.size()) / data.size();
        std::cout << "  Level " << level << ": " 
                  << compressed.size() << " / " << data.size() 
                  << " bytes (ratio: " << (ratio * 100.0) << "%)\n";
    }
    
    std::cout << "  ✓ ZLIB compression test passed\n\n";
}

void test_compress_with_info() {
    std::cout << "Testing compress with info...\n";
    
    if (!Compression::has_zlib_support()) {
        std::cout << "  ⚠ ZLIB not available, skipping...\n\n";
        return;
    }
    
    // Create test data
    std::vector<uint8_t> data(10000, 0x42);
    
    auto [compressed, info] = Compression::compress_with_info(
        data, Compression::Algorithm::ZLIB, 6);
    
    assert(info.original_size == 10000);
    assert(info.compressed_size == compressed.size());
    assert(info.compression_ratio == 
           static_cast<double>(compressed.size()) / 10000.0);
    assert(info.algorithm == Compression::Algorithm::ZLIB);
    assert(info.level == 6);
    
    std::cout << "  Original size: " << info.original_size << " bytes\n";
    std::cout << "  Compressed size: " << info.compressed_size << " bytes\n";
    std::cout << "  Compression ratio: " << (info.compression_ratio * 100.0) << "%\n";
    
    std::cout << "  ✓ Compress with info test passed\n\n";
}

void test_is_zlib_compressed() {
    std::cout << "Testing ZLIB detection...\n";
    
    // ZLIB compressed data (magic bytes: 0x78 followed by checksum)
    std::vector<uint8_t> zlib_data = {0x78, 0x9c, 0x00, 0x00, 0x00, 0x00, 0x01};
    assert(Compression::is_zlib_compressed(zlib_data));
    
    // Non-ZLIB data
    std::vector<uint8_t> non_zlib_data = {0x01, 0x02, 0x03, 0x04, 0x05};
    assert(!Compression::is_zlib_compressed(non_zlib_data));
    
    // Empty data
    std::vector<uint8_t> empty_data;
    assert(!Compression::is_zlib_compressed(empty_data));
    
    std::cout << "  ✓ ZLIB detection test passed\n\n";
}

void test_estimate_compression_ratio() {
    std::cout << "Testing compression ratio estimation...\n";
    
    // Highly compressible data (all same byte)
    std::vector<uint8_t> uniform_data(1000, 0x42);
    double ratio1 = Compression::estimate_compression_ratio(uniform_data);
    std::cout << "  Uniform data ratio: " << ratio1 << "\n";
    assert(ratio1 < 0.5);  // Should be very compressible
    
    // Random-looking data
    std::vector<uint8_t> random_data;
    for (int i = 0; i < 1000; ++i) {
        random_data.push_back(rand() % 256);
    }
    double ratio2 = Compression::estimate_compression_ratio(random_data);
    std::cout << "  Random data ratio: " << ratio2 << "\n";
    assert(ratio2 > ratio1);  // Should be less compressible
    
    std::cout << "  ✓ Compression ratio estimation test passed\n\n";
}

void test_recommended_level() {
    std::cout << "Testing recommended compression level...\n";
    
    // Test different sizes
    int level1 = Compression::get_recommended_level(100);       // < 1 KB
    int level2 = Compression::get_recommended_level(5000);      // < 10 KB
    int level3 = Compression::get_recommended_level(50000);     // < 100 KB
    int level4 = Compression::get_recommended_level(500000);    // < 1 MB
    int level5 = Compression::get_recommended_level(2000000);   // >= 1 MB
    
    std::cout << "  100 bytes -> level " << level1 << "\n";
    std::cout << "  5 KB -> level " << level2 << "\n";
    std::cout << "  50 KB -> level " << level3 << "\n";
    std::cout << "  500 KB -> level " << level4 << "\n";
    std::cout << "  2 MB -> level " << level5 << "\n";
    
    assert(level1 <= level2);
    assert(level2 <= level3);
    assert(level3 <= level4);
    assert(level4 <= level5);
    
    std::cout << "  ✓ Recommended level test passed\n\n";
}

void test_zlib_version() {
    std::cout << "Testing ZLIB version...\n";
    
    std::string version = Compression::get_zlib_version();
    std::cout << "  ZLIB version: " << version << "\n";
    
    if (Compression::has_zlib_support()) {
        assert(version != "zlib not available");
    } else {
        assert(version == "zlib not available");
    }
    
    std::cout << "  ✓ ZLIB version test passed\n\n";
}

void test_compress_complex() {
    std::cout << "Testing compression of simplicial complex...\n";
    
    if (!Compression::has_zlib_support()) {
        std::cout << "  ⚠ ZLIB not available, skipping...\n\n";
        return;
    }
    
    // Create a large complex
    SimplicialComplex complex;
    const int grid_size = 50;
    
    for (int i = 0; i < grid_size; ++i) {
        for (int j = 0; j < grid_size; ++j) {
            complex.add_vertex();
        }
    }
    
    std::cout << "  Created complex with " << complex.vertex_count() << " vertices\n";
    
    // Serialize
    auto binary = BinarySerializer::serialize(complex);
    std::cout << "  Binary size: " << binary.size() << " bytes\n";
    
    // Compress
    auto compressed = Compression::compress_zlib(binary, 6);
    std::cout << "  Compressed size: " << compressed.size() << " bytes\n";
    std::cout << "  Compression ratio: " 
              << (static_cast<double>(compressed.size()) / binary.size() * 100.0) 
              << "%\n";
    
    // Decompress
    auto decompressed = Compression::decompress_zlib(compressed);
    assert(decompressed == binary);
    
    std::cout << "  ✓ Complex compression test passed\n\n";
}

void test_compression_performance() {
    std::cout << "Testing compression performance...\n";
    
    if (!Compression::has_zlib_support()) {
        std::cout << "  ⚠ ZLIB not available, skipping...\n\n";
        return;
    }
    
    // Create large data
    std::vector<uint8_t> data(100000);
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<uint8_t>(i % 256);
    }
    
    std::cout << "  Data size: " << data.size() << " bytes\n";
    
    // Measure compression time
    auto start = std::chrono::high_resolution_clock::now();
    auto compressed = Compression::compress_zlib(data, 6);
    auto end = std::chrono::high_resolution_clock::now();
    auto compress_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        end - start).count();
    
    std::cout << "  Compression time: " << compress_time << " ms\n";
    
    // Measure decompression time
    start = std::chrono::high_resolution_clock::now();
    auto decompressed = Compression::decompress_zlib(compressed);
    end = std::chrono::high_resolution_clock::now();
    auto decompress_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        end - start).count();
    
    std::cout << "  Decompression time: " << decompress_time << " ms\n";
    
    assert(decompressed == data);
    
    std::cout << "  ✓ Compression performance test passed\n\n";
}

int main() {
    std::cout << "\n=== Compression Tests ===\n\n";
    
    try {
        test_no_compression();
        test_zlib_compression();
        test_compress_with_info();
        test_is_zlib_compressed();
        test_estimate_compression_ratio();
        test_recommended_level();
        test_zlib_version();
        test_compress_complex();
        test_compression_performance();
        
        std::cout << "=== All Compression Tests Passed! ===\n\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
