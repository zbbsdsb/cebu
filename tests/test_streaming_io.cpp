#include <cebu/streaming_io.h>
#include <cebu/json_serialization.h>
#include <gtest/gtest.h>
#include <fstream>

using namespace cebu;

class StreamingIOTest : public ::testing::Test {
protected:
    void SetUp() override {
        build_test_complex();
        save_test_file();
    }

    void TearDown() override {
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

        complex.add_face(v0, v1, v2);
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
TEST_F(StreamingIOTest, StreamLoad) {
    StreamingLoader loader("test_complex.json");

    auto loaded_complex = loader.load();

    EXPECT_EQ(loaded_complex.vertex_count(), complex.vertex_count());
    EXPECT_EQ(loaded_complex.simplex_count(), complex.simplex_count());
}

// 2. Stream write test
TEST_F(StreamingIOTest, StreamWrite) {
    StreamingWriter writer("test_output.json");

    bool written = writer.write(complex);

    EXPECT_TRUE(written);

    // Verify file exists
    std::ifstream in("test_output.json");
    EXPECT_TRUE(in.good());
    in.close();

    // Load and verify
    StreamingLoader loader("test_output.json");
    auto loaded = loader.load();

    EXPECT_EQ(loaded.vertex_count(), complex.vertex_count());
}

// 3. Progress callback test
TEST_F(StreamingIOTest, ProgressCallback) {
    bool callback_called = false;
    size_t callback_count = 0;

    StreamingLoader loader("test_complex.json");

    loader.set_progress_callback([&](size_t current, size_t total) {
        callback_called = true;
        callback_count++;
        EXPECT_LE(current, total);
    });

    auto loaded = loader.load();

    EXPECT_TRUE(callback_called);
    EXPECT_GT(callback_count, 0);
}

// 4. File metadata test
TEST_F(StreamingIOTest, FileMetadata) {
    StreamingLoader loader("test_complex.json");

    size_t total_simplices = loader.get_total_simplices();
    size_t total_vertices = loader.get_total_vertices();
    size_t max_dim = loader.get_max_dimension();
    size_t file_size = loader.get_file_size();

    EXPECT_GT(total_simplices, 0);
    EXPECT_GT(total_vertices, 0);
    EXPECT_GE(max_dim, 0);
    EXPECT_GT(file_size, 0);
}

// 5. Compressed file test
TEST_F(StreamingIOTest, CompressedFile) {
    // Create compressed file
    PersistenceOptions options;
    options.compression = Compression::ZLIB;
    options.compression_level = 6;

    Persistence::save(complex, "test_compressed.bin", options);

    StreamingLoader loader("test_compressed.bin");

    bool is_compressed = loader.is_compressed();

    EXPECT_TRUE(is_compressed);

    std::remove("test_compressed.bin");
}

// 6. Non-existent file test
TEST_F(StreamingIOTest, NonExistentFile) {
    EXPECT_THROW(
        StreamingLoader loader("non_existent_file.json"),
        std::runtime_error
    );
}

// 7. Stream labeled test
TEST_F(StreamingIOTest, StreamLabeled) {
    SimplicialComplexLabeled<double> labeled;

    VertexID v0 = labeled.add_vertex();
    VertexID v1 = labeled.add_vertex();
    EdgeID e0 = labeled.add_edge(v0, v1);

    labeled.set_label(v0, 0.5);
    labeled.set_label(v1, 0.8);
    labeled.set_label(e0, 0.7);

    StreamingWriter writer("test_labeled.json");
    bool written = writer.write_labeled(labeled);

    EXPECT_TRUE(written);

    StreamingLoader loader("test_labeled.json");
    auto loaded = loader.load_labeled<double>();

    EXPECT_EQ(loaded.vertex_count(), labeled.vertex_count());
    EXPECT_EQ(loaded.simplex_count(), labeled.simplex_count());

    std::remove("test_labeled.json");
}

// 8. Compression settings test
TEST_F(StreamingIOTest, CompressionSettings) {
    StreamingWriter writer("test_output.json");

    writer.set_compression(false);
    EXPECT_NO_THROW(writer.write(complex));

    writer.set_compression(true, 9);

    if (Compression::has_zlib_support()) {
        EXPECT_NO_THROW(writer.write(complex));
    }
}

// 9. Chunk loading test
TEST_F(StreamingIOTest, ChunkLoading) {
    ChunkedStreamingLoader loader("test_complex.json", 1024);

    auto chunks = loader.get_chunks();

    EXPECT_GT(chunks.size(), 0);

    // Load first chunk
    if (chunks.size() > 0) {
        auto chunk_data = loader.load_chunk(0);
        EXPECT_TRUE(chunk_data.contains("format"));
    }
}

// 10. Empty file test
TEST_F(StreamingIOTest, EmptyFile) {
    // Create empty JSON file
    std::ofstream out("empty.json");
    out << "{}";
    out.close();

    EXPECT_THROW(
        StreamingLoader loader("empty.json"),
        std::exception
    );

    std::remove("empty.json");
}

// 11. Large file metadata test
TEST_F(StreamingIOTest, LargeFileMetadata) {
    StreamingLoader loader("test_complex.json");

    size_t file_size = loader.get_file_size();
    size_t vertices = loader.get_total_vertices();
    size_t simplices = loader.get_total_simplices();

    // Verify relationships
    EXPECT_LE(vertices, simplices);
    EXPECT_GT(file_size, 0);
}

// 12. Multiple writes test
TEST_F(StreamingIOTest, MultipleWrites) {
    StreamingWriter writer("test_output.json");

    bool written1 = writer.write(complex);
    EXPECT_TRUE(written1);

    bool written2 = writer.write(complex);
    EXPECT_TRUE(written2);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
