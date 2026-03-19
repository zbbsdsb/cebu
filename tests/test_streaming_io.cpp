// Copyright (c) 2026 CeaserZhao (Oasis Company)
// SPDX-License-Identifier: MIT
#include <cebu/streaming_io.h>
#include <cebu/json_serialization.h>
#include <cebu/persistence.h>
#include <fstream>
#include <cassert>
#include <iostream>

using namespace cebu;

class StreamingIOTest {
public:
    StreamingIOTest() {
        build_test_complex();
        save_test_file();
    }

    ~StreamingIOTest() {
        std::remove("test_complex.json");
        std::remove("test_output.json");
    }

    void build_test_complex() {
        VertexID v0 = complex.add_vertex();
        VertexID v1 = complex.add_vertex();
        VertexID v2 = complex.add_vertex();

        complex.add_edge(v0, v1);
        complex.add_edge(v1, v2);
        complex.add_edge(v2, v0);

        complex.add_triangle(v0, v1, v2);
    }

    void save_test_file() {
        nlohmann::json j = JsonSerializer::serialize(complex);
        std::ofstream out("test_complex.json");
        out << j.dump(2);
        out.close();
    }

    SimplicialComplex complex;
};

// 1. Stream load test
void test_stream_load() {
    std::cout << "Test 1: Stream Load... " << std::endl;
    StreamingIOTest test;
    
    StreamingLoader loader("test_complex.json");

    auto loaded_complex = loader.load();

    assert(loaded_complex.vertex_count() == test.complex.vertex_count());
    assert(loaded_complex.simplex_count() == test.complex.simplex_count());
    
    std::cout << "âœ?Stream load test passed" << std::endl;
}

// 2. Stream write test
void test_stream_write() {
    std::cout << "Test 2: Stream Write... " << std::endl;
    StreamingIOTest test;
    
    StreamingWriter writer("test_output.json");

    bool written = writer.write(test.complex);

    assert(written);

    // Verify file exists
    std::ifstream in("test_output.json");
    assert(in.good());
    in.close();

    // Load and verify
    StreamingLoader loader("test_output.json");
    auto loaded = loader.load();

    assert(loaded.vertex_count() == test.complex.vertex_count());
    
    std::cout << "âœ?Stream write test passed" << std::endl;
}

// 3. Progress callback test
void test_progress_callback() {
    std::cout << "Test 3: Progress Callback... " << std::endl;
    StreamingIOTest test;
    
    bool callback_called = false;
    size_t callback_count = 0;

    StreamingLoader loader("test_complex.json");

    loader.set_progress_callback([&](size_t current, size_t total) {
        callback_called = true;
        callback_count++;
        assert(current <= total);
    });

    auto loaded = loader.load();

    assert(callback_called);
    assert(callback_count > 0);
    
    std::cout << "âœ?Progress callback test passed" << std::endl;
}

// 4. File metadata test
void test_file_metadata() {
    std::cout << "Test 4: File Metadata... " << std::endl;
    StreamingIOTest test;
    
    StreamingLoader loader("test_complex.json");

    size_t total_simplices = loader.get_total_simplices();
    size_t total_vertices = loader.get_total_vertices();
    size_t max_dim = loader.get_max_dimension();
    size_t file_size = loader.get_file_size();

    assert(total_simplices > 0);
    assert(total_vertices > 0);
    assert(max_dim >= 0);
    assert(file_size > 0);
    
    std::cout << "âœ?File metadata test passed" << std::endl;
}

// 5. Compressed file test
void test_compressed_file() {
    std::cout << "Test 5: Compressed File... " << std::endl;
    StreamingIOTest test;
    
    // Create compressed file
    cebu::PersistenceOptions options;
    options.compression = cebu::Compression::Algorithm::ZLIB;
    options.compression_level = 6;

    cebu::Persistence::save(test.complex, "test_compressed.bin", options);

    StreamingLoader loader("test_compressed.bin");

    bool is_compressed = loader.is_compressed();

    if (Compression::has_zlib_support()) {
        assert(is_compressed);
    }

    std::remove("test_compressed.bin");
    
    std::cout << "âœ?Compressed file test passed" << std::endl;
}

// 6. Non-existent file test
void test_non_existent_file() {
    std::cout << "Test 6: Non-Existent File... " << std::endl;
    
    bool threw = false;
    try {
        StreamingLoader loader("non_existent_file.json");
    } catch (const std::runtime_error&) {
        threw = true;
    }
    assert(threw);
    
    std::cout << "âœ?Non-existent file test passed" << std::endl;
}

// 7. Stream labeled test
void test_stream_labeled() {
    std::cout << "Test 7: Stream Labeled... " << std::endl;
    StreamingIOTest test;
    
    SimplicialComplexLabeled<double> labeled;

    VertexID v0 = labeled.add_vertex();
    VertexID v1 = labeled.add_vertex();
    SimplexID e0 = labeled.add_edge(v0, v1);

    labeled.set_label(v0, 0.5);
    labeled.set_label(v1, 0.8);
    labeled.set_label(e0, 0.7);

    StreamingWriter writer("test_labeled.json");
    bool written = writer.write_labeled(labeled);

    assert(written);

    StreamingLoader loader("test_labeled.json");
    auto loaded = loader.load_labeled<double>();

    assert(loaded.vertex_count() == labeled.vertex_count());
    assert(loaded.simplex_count() == labeled.simplex_count());

    std::remove("test_labeled.json");
    
    std::cout << "âœ?Stream labeled test passed" << std::endl;
}

// 8. Compression settings test
void test_compression_settings() {
    std::cout << "Test 8: Compression Settings... " << std::endl;
    StreamingIOTest test;
    
    StreamingWriter writer("test_output.json");

    writer.set_compression(false);
    try {
        writer.write(test.complex);
    } catch (...) {
        assert(false);
    }

    writer.set_compression(true, 9);

    if (Compression::has_zlib_support()) {
        try {
            writer.write(test.complex);
        } catch (...) {
            assert(false);
        }
    }
    
    std::cout << "âœ?Compression settings test passed" << std::endl;
}

// 9. Chunk loading test
void test_chunk_loading() {
    std::cout << "Test 9: Chunk Loading... " << std::endl;
    StreamingIOTest test;
    
    ChunkedStreamingLoader loader("test_complex.json", 1024);

    auto chunks = loader.get_chunks();

    assert(chunks.size() > 0);

    // Load first chunk
    if (chunks.size() > 0) {
        auto chunk_data = loader.load_chunk(0);
        assert(chunk_data.contains("format"));
    }
    
    std::cout << "âœ?Chunk loading test passed" << std::endl;
}

// 10. Empty file test
void test_empty_file() {
    std::cout << "Test 10: Empty File... " << std::endl;
    
    // Create empty JSON file
    std::ofstream out("empty.json");
    out << "{}";
    out.close();

    bool threw = false;
    try {
        StreamingLoader loader("empty.json");
    } catch (const std::exception&) {
        threw = true;
    }
    assert(threw);

    std::remove("empty.json");
    
    std::cout << "âœ?Empty file test passed" << std::endl;
}

// 11. Large file metadata test
void test_large_file_metadata() {
    std::cout << "Test 11: Large File Metadata... " << std::endl;
    StreamingIOTest test;
    
    StreamingLoader loader("test_complex.json");

    size_t file_size = loader.get_file_size();
    size_t vertices = loader.get_total_vertices();
    size_t simplices = loader.get_total_simplices();

    // Verify relationships
    assert(vertices <= simplices);
    assert(file_size > 0);
    
    std::cout << "âœ?Large file metadata test passed" << std::endl;
}

// 12. Multiple writes test
void test_multiple_writes() {
    std::cout << "Test 12: Multiple Writes... " << std::endl;
    StreamingIOTest test;
    
    StreamingWriter writer("test_output.json");

    bool written1 = writer.write(test.complex);
    assert(written1);

    bool written2 = writer.write(test.complex);
    assert(written2);
    
    std::cout << "âœ?Multiple writes test passed" << std::endl;
}

int main() {
    std::cout << "=== Streaming IO Tests ===" << std::endl << std::endl;
    
    try {
        test_stream_load();
        test_stream_write();
        test_progress_callback();
        test_file_metadata();
        test_compressed_file();
        test_non_existent_file();
        test_stream_labeled();
        test_compression_settings();
        test_chunk_loading();
        test_empty_file();
        test_large_file_metadata();
        test_multiple_writes();
        
        std::cout << std::endl;
        std::cout << "=== All Streaming IO Tests Passed! ===" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
