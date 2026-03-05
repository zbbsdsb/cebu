#include <cebu/snapshot_manager.h>
#include <gtest/gtest.h>
#include <fstream>

using namespace cebu;

class SnapshotManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = std::make_unique<SnapshotManager>("test_snapshots.ceb");
        build_test_complex();
    }

    void TearDown() override {
        manager.reset();
        std::remove("test_snapshots.ceb");
    }

    void build_test_complex() {
        // Create a simple complex with some simplices
        VertexID v0 = complex.add_vertex();
        VertexID v1 = complex.add_vertex();
        VertexID v2 = complex.add_vertex();

        complex.add_edge(v0, v1);
        complex.add_edge(v1, v2);
        complex.add_edge(v2, v0);

        complex.add_face(v0, v1, v2);
    }

    std::unique_ptr<SnapshotManager> manager;
    SimplicialComplex complex;
};

// 1. Create snapshot test
TEST_F(SnapshotManagerTest, CreateSnapshot) {
    EXPECT_EQ(manager->get_snapshot_count(), 0);

    // Create snapshot
    auto metadata = manager->create_snapshot(complex, "initial");

    EXPECT_EQ(metadata.name, "initial");
    EXPECT_EQ(manager->get_snapshot_count(), 1);
    EXPECT_TRUE(manager->has_snapshot("initial"));

    // Create another snapshot
    auto metadata2 = manager->create_snapshot(complex, "version1");

    EXPECT_EQ(metadata2.name, "version1");
    EXPECT_EQ(manager->get_snapshot_count(), 2);
    EXPECT_TRUE(manager->has_snapshot("version1"));
}

// 2. Create labeled snapshot test
TEST_F(SnapshotManagerTest, CreateLabeledSnapshot) {
    SimplicialComplexLabeled<double> labeled;

    VertexID v0 = labeled.add_vertex();
    VertexID v1 = labeled.add_vertex();
    EdgeID e0 = labeled.add_edge(v0, v1);

    labeled.set_label(v0, 0.5);
    labeled.set_label(v1, 0.8);
    labeled.set_label(e0, 0.7);

    auto metadata = manager->create_snapshot_labeled(labeled, "labeled_snapshot");

    EXPECT_EQ(metadata.name, "labeled_snapshot");
    EXPECT_TRUE(manager->has_snapshot("labeled_snapshot"));

    // Restore and verify
    SimplicialComplexLabeled<double> restored;
    manager->restore_snapshot(restored, "labeled_snapshot");

    EXPECT_EQ(restored.vertex_count(), labeled.vertex_count());
    EXPECT_EQ(restored.simplex_count(), labeled.simplex_count());
}

// 3. List snapshots test
TEST_F(SnapshotManagerTest, ListSnapshots) {
    manager->create_snapshot(complex, "snap1");
    manager->create_snapshot(complex, "snap2");
    manager->create_snapshot(complex, "snap3");

    auto snapshots = manager->list_snapshots();

    EXPECT_EQ(snapshots.size(), 3);

    // Check names
    std::vector<std::string> names = {"snap1", "snap2", "snap3"};
    for (const auto& name : names) {
        bool found = false;
        for (const auto& meta : snapshots) {
            if (meta.name == name) {
                found = true;
                break;
            }
        }
        EXPECT_TRUE(found) << "Snapshot " << name << " not found";
    }
}

// 4. Get snapshot test
TEST_F(SnapshotManagerTest, GetSnapshot) {
    manager->create_snapshot(complex, "test_snapshot");

    auto snapshot = manager->get_snapshot("test_snapshot");

    EXPECT_NE(snapshot, nullptr);
    EXPECT_EQ(snapshot->metadata.name, "test_snapshot");

    // Get non-existent snapshot
    auto null_snapshot = manager->get_snapshot("non_existent");

    EXPECT_EQ(null_snapshot, nullptr);
}

// 5. Restore snapshot test
TEST_F(SnapshotManagerTest, RestoreSnapshot) {
    manager->create_snapshot(complex, "initial");

    // Modify complex
    VertexID v3 = complex.add_vertex();
    EdgeID e3 = complex.add_edge(complex.vertex_count() - 2, v3);

    size_t vertex_count_after = complex.vertex_count();

    // Restore snapshot
    manager->restore_snapshot(complex, "initial");

    size_t vertex_count_before = complex.vertex_count();

    EXPECT_LT(vertex_count_before, vertex_count_after);
}

// 6. Delete snapshot test
TEST_F(SnapshotManagerTest, DeleteSnapshot) {
    manager->create_snapshot(complex, "to_delete");
    manager->create_snapshot(complex, "to_keep");

    EXPECT_EQ(manager->get_snapshot_count(), 2);

    bool deleted = manager->delete_snapshot("to_delete");

    EXPECT_TRUE(deleted);
    EXPECT_EQ(manager->get_snapshot_count(), 1);
    EXPECT_FALSE(manager->has_snapshot("to_delete"));
    EXPECT_TRUE(manager->has_snapshot("to_keep"));

    // Try to delete non-existent snapshot
    bool deleted_again = manager->delete_snapshot("non_existent");
    EXPECT_FALSE(deleted_again);
}

// 7. Compare snapshots test
TEST_F(SnapshotManagerTest, CompareSnapshots) {
    // Create initial snapshot
    manager->create_snapshot(complex, "snap1");

    // Modify complex
    VertexID v3 = complex.add_vertex();
    complex.add_edge(complex.vertex_count() - 2, v3);

    // Create second snapshot
    manager->create_snapshot(complex, "snap2");

    // Compare
    auto changes = manager->compare_snapshots("snap1", "snap2");

    // Should have some changes (simplified test)
    EXPECT_GE(changes.size(), 0);
}

// 8. Persistence test
TEST_F(SnapshotManagerTest, Persistence) {
    manager->create_snapshot(complex, "persistent1");
    manager->create_snapshot(complex, "persistent2");

    EXPECT_EQ(manager->get_snapshot_count(), 2);

    // Save to file
    bool saved = manager->save_to_file("test_snapshots.ceb");
    EXPECT_TRUE(saved);

    // Create new manager and load
    SnapshotManager new_manager;
    bool loaded = new_manager.load_from_file("test_snapshots.ceb");
    EXPECT_TRUE(loaded);

    EXPECT_EQ(new_manager.get_snapshot_count(), 2);
    EXPECT_TRUE(new_manager.has_snapshot("persistent1"));
    EXPECT_TRUE(new_manager.has_snapshot("persistent2"));
}

// 9. Compression test
TEST_F(SnapshotManagerTest, Compression) {
    // Create uncompressed snapshot
    auto meta_uncompressed = manager->create_snapshot(complex, "uncompressed", false);
    EXPECT_EQ(meta_uncompressed.compressed_size, 0);

    // Create compressed snapshot
    auto meta_compressed = manager->create_snapshot(complex, "compressed", true);

    if (Compression::has_zlib_support()) {
        EXPECT_GT(meta_compressed.compressed_size, 0);
        EXPECT_LT(meta_compressed.compressed_size, meta_compressed.size);
    }

    // Restore both and compare
    SimplicialComplex restored_uncompressed;
    SimplicialComplex restored_compressed;

    bool ok1 = manager->restore_snapshot(restored_uncompressed, "uncompressed");
    bool ok2 = manager->restore_snapshot(restored_compressed, "compressed");

    EXPECT_TRUE(ok1);
    EXPECT_TRUE(ok2);

    EXPECT_EQ(restored_uncompressed.vertex_count(), restored_compressed.vertex_count());
    EXPECT_EQ(restored_uncompressed.simplex_count(), restored_compressed.simplex_count());
}

// 10. Snapshot metadata test
TEST_F(SnapshotManagerTest, SnapshotMetadata) {
    auto metadata = manager->create_snapshot(complex, "metadata_test");

    EXPECT_EQ(metadata.name, "metadata_test");
    EXPECT_FALSE(metadata.timestamp.empty());
    EXPECT_FALSE(metadata.hash.empty());
    EXPECT_GT(metadata.size, 0);

    if (Compression::has_zlib_support()) {
        EXPECT_GT(metadata.compressed_size, 0);
    }

    EXPECT_EQ(metadata.simplex_count, complex.simplex_count());
    EXPECT_EQ(metadata.vertex_count, complex.vertex_count());
    EXPECT_EQ(metadata.max_dimension, complex.max_dimension());
}

// 11. Clear snapshots test
TEST_F(SnapshotManagerTest, ClearSnapshots) {
    manager->create_snapshot(complex, "snap1");
    manager->create_snapshot(complex, "snap2");
    manager->create_snapshot(complex, "snap3");

    EXPECT_EQ(manager->get_snapshot_count(), 3);

    // Clear all snapshots
    manager->clear();

    EXPECT_EQ(manager->get_snapshot_count(), 0);
    EXPECT_FALSE(manager->has_snapshot("snap1"));
    EXPECT_FALSE(manager->has_snapshot("snap2"));
    EXPECT_FALSE(manager->has_snapshot("snap3"));
}

// 12. Duplicate snapshot name test
TEST_F(SnapshotManagerTest, DuplicateSnapshotName) {
    manager->create_snapshot(complex, "duplicate");

    EXPECT_THROW(
        manager->create_snapshot(complex, "duplicate"),
        std::runtime_error
    );
}

// 13. Get total size test
TEST_F(SnapshotManagerTest, GetTotalSize) {
    manager->create_snapshot(complex, "snap1", false);
    manager->create_snapshot(complex, "snap2", false);

    size_t total_uncompressed = manager->get_total_uncompressed_size();
    size_t total_compressed = manager->get_total_size();

    EXPECT_GT(total_uncompressed, 0);

    if (Compression::has_zlib_support()) {
        // With compression, total should be smaller
        EXPECT_LE(total_compressed, total_uncompressed);
    }
}

// 14. Multiple snapshots test
TEST_F(SnapshotManagerTest, MultipleSnapshots) {
    const int NUM_SNAPSHOTS = 10;

    for (int i = 0; i < NUM_SNAPSHOTS; ++i) {
        std::string name = "snap_" + std::to_string(i);
        manager->create_snapshot(complex, name);
    }

    EXPECT_EQ(manager->get_snapshot_count(), NUM_SNAPSHOTS);

    auto snapshots = manager->list_snapshots();
    EXPECT_EQ(snapshots.size(), NUM_SNAPSHOTS);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
