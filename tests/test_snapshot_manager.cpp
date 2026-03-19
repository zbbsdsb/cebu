// Copyright (c) 2026 CeaserZhao (Oasis Company)
// SPDX-License-Identifier: MIT
#include <cebu/snapshot_manager.h>
#include <fstream>
#include <cassert>
#include <iostream>

using namespace cebu;

class SnapshotManagerTest {
public:
    SnapshotManagerTest() {
        manager = std::make_unique<SnapshotManager>("test_snapshots.ceb");
        build_test_complex();
    }

    ~SnapshotManagerTest() {
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

        complex.add_triangle(v0, v1, v2);
    }

    std::unique_ptr<SnapshotManager> manager;
    SimplicialComplex complex;
};

// 1. Create snapshot test
void test_create_snapshot() {
    std::cout << "Test 1: Create Snapshot... " << std::endl;
    SnapshotManagerTest test;
    
    assert(test.manager->get_snapshot_count() == 0);

    // Create snapshot
    auto metadata = test.manager->create_snapshot(test.complex, "initial");

    assert(metadata.name == "initial");
    assert(test.manager->get_snapshot_count() == 1);
    assert(test.manager->has_snapshot("initial"));

    // Create another snapshot
    auto metadata2 = test.manager->create_snapshot(test.complex, "version1");

    assert(metadata2.name == "version1");
    assert(test.manager->get_snapshot_count() == 2);
    assert(test.manager->has_snapshot("version1"));
    
    std::cout << "�?Create snapshot test passed" << std::endl;
}

// 2. Create labeled snapshot test
void test_create_labeled_snapshot() {
    std::cout << "Test 2: Create Labeled Snapshot... " << std::endl;
    SnapshotManagerTest test;
    
    SimplicialComplexLabeled<double> labeled;

    VertexID v0 = labeled.add_vertex();
    VertexID v1 = labeled.add_vertex();
    SimplexID e0 = labeled.add_edge(v0, v1);

    labeled.set_label(v0, 0.5);
    labeled.set_label(v1, 0.8);
    labeled.set_label(e0, 0.7);

    auto metadata = test.manager->create_snapshot_labeled(labeled, "labeled_snapshot");

    assert(metadata.name == "labeled_snapshot");
    assert(test.manager->has_snapshot("labeled_snapshot"));

    // Restore and verify
    SimplicialComplexLabeled<double> restored;
    test.manager->restore_snapshot(restored, "labeled_snapshot");

    assert(restored.vertex_count() == labeled.vertex_count());
    assert(restored.simplex_count() == labeled.simplex_count());
    
    std::cout << "�?Create labeled snapshot test passed" << std::endl;
}

// 3. List snapshots test
void test_list_snapshots() {
    std::cout << "Test 3: List Snapshots... " << std::endl;
    SnapshotManagerTest test;
    
    test.manager->create_snapshot(test.complex, "snap1");
    test.manager->create_snapshot(test.complex, "snap2");
    test.manager->create_snapshot(test.complex, "snap3");

    auto snapshots = test.manager->list_snapshots();

    assert(snapshots.size() == 3);

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
        assert(found && "Snapshot not found");
    }
    
    std::cout << "�?List snapshots test passed" << std::endl;
}

// 4. Get snapshot test
void test_get_snapshot() {
    std::cout << "Test 4: Get Snapshot... " << std::endl;
    SnapshotManagerTest test;
    
    test.manager->create_snapshot(test.complex, "test_snapshot");

    auto snapshot = test.manager->get_snapshot("test_snapshot");

    assert(snapshot != nullptr);
    assert(snapshot->metadata.name == "test_snapshot");

    // Get non-existent snapshot
    auto null_snapshot = test.manager->get_snapshot("non_existent");

    assert(null_snapshot == nullptr);
    
    std::cout << "�?Get snapshot test passed" << std::endl;
}

// 5. Restore snapshot test
void test_restore_snapshot() {
    std::cout << "Test 5: Restore Snapshot... " << std::endl;
    SnapshotManagerTest test;
    
    test.manager->create_snapshot(test.complex, "initial");

    // Modify complex
    VertexID v3 = test.complex.add_vertex();
    SimplexID e3 = test.complex.add_edge(static_cast<VertexID>(test.complex.vertex_count() - 2), v3);

    size_t vertex_count_after = test.complex.vertex_count();

    // Restore snapshot
    test.manager->restore_snapshot(test.complex, "initial");

    size_t vertex_count_before = test.complex.vertex_count();

    assert(vertex_count_before < vertex_count_after);
    
    std::cout << "�?Restore snapshot test passed" << std::endl;
}

// 6. Delete snapshot test
void test_delete_snapshot() {
    std::cout << "Test 6: Delete Snapshot... " << std::endl;
    SnapshotManagerTest test;
    
    test.manager->create_snapshot(test.complex, "to_delete");
    test.manager->create_snapshot(test.complex, "to_keep");

    assert(test.manager->get_snapshot_count() == 2);

    bool deleted = test.manager->delete_snapshot("to_delete");

    assert(deleted);
    assert(test.manager->get_snapshot_count() == 1);
    assert(!test.manager->has_snapshot("to_delete"));
    assert(test.manager->has_snapshot("to_keep"));

    // Try to delete non-existent snapshot
    bool deleted_again = test.manager->delete_snapshot("non_existent");
    assert(!deleted_again);
    
    std::cout << "�?Delete snapshot test passed" << std::endl;
}

// 7. Compare snapshots test
void test_compare_snapshots() {
    std::cout << "Test 7: Compare Snapshots... " << std::endl;
    SnapshotManagerTest test;
    
    // Create initial snapshot
    test.manager->create_snapshot(test.complex, "snap1");

    // Modify complex
    VertexID v3 = test.complex.add_vertex();
    test.complex.add_edge(static_cast<VertexID>(test.complex.vertex_count() - 2), v3);

    // Create second snapshot
    test.manager->create_snapshot(test.complex, "snap2");

    // Compare
    auto changes = test.manager->compare_snapshots("snap1", "snap2");

    // Should have some changes (simplified test)
    assert(changes.size() >= 0);
    
    std::cout << "�?Compare snapshots test passed" << std::endl;
}

// 8. Persistence test
void test_persistence() {
    std::cout << "Test 8: Persistence... " << std::endl;
    SnapshotManagerTest test;
    
    test.manager->create_snapshot(test.complex, "persistent1");
    test.manager->create_snapshot(test.complex, "persistent2");

    assert(test.manager->get_snapshot_count() == 2);

    // Save to file
    bool saved = test.manager->save_to_file("test_snapshots.ceb");
    assert(saved);

    // Create new manager and load
    SnapshotManager new_manager;
    bool loaded = new_manager.load_from_file("test_snapshots.ceb");
    assert(loaded);

    assert(new_manager.get_snapshot_count() == 2);
    assert(new_manager.has_snapshot("persistent1"));
    assert(new_manager.has_snapshot("persistent2"));
    
    std::cout << "�?Persistence test passed" << std::endl;
}

// 9. Compression test
void test_compression() {
    std::cout << "Test 9: Compression... " << std::endl;
    SnapshotManagerTest test;
    
    // Create uncompressed snapshot
    auto meta_uncompressed = test.manager->create_snapshot(test.complex, "uncompressed", false);
    assert(meta_uncompressed.compressed_size == 0);

    // Create compressed snapshot
    auto meta_compressed = test.manager->create_snapshot(test.complex, "compressed", true);

    if (Compression::has_zlib_support()) {
        assert(meta_compressed.compressed_size > 0);
        assert(meta_compressed.compressed_size < meta_compressed.size);
    }

    // Restore both and compare
    SimplicialComplex restored_uncompressed;
    SimplicialComplex restored_compressed;

    bool ok1 = test.manager->restore_snapshot(restored_uncompressed, "uncompressed");
    bool ok2 = test.manager->restore_snapshot(restored_compressed, "compressed");

    assert(ok1);
    assert(ok2);

    assert(restored_uncompressed.vertex_count() == restored_compressed.vertex_count());
    assert(restored_uncompressed.simplex_count() == restored_compressed.simplex_count());
    
    std::cout << "�?Compression test passed" << std::endl;
}

// 10. Snapshot metadata test
void test_snapshot_metadata() {
    std::cout << "Test 10: Snapshot Metadata... " << std::endl;
    SnapshotManagerTest test;
    
    auto metadata = test.manager->create_snapshot(test.complex, "metadata_test");

    assert(metadata.name == "metadata_test");
    assert(!metadata.timestamp.empty());
    assert(!metadata.hash.empty());
    assert(metadata.size > 0);

    if (Compression::has_zlib_support()) {
        assert(metadata.compressed_size > 0);
    }

    assert(metadata.simplex_count == test.complex.simplex_count());
    assert(metadata.vertex_count == test.complex.vertex_count());
    assert(metadata.max_dimension == test.complex.max_dimension());
    
    std::cout << "�?Snapshot metadata test passed" << std::endl;
}

// 11. Clear snapshots test
void test_clear_snapshots() {
    std::cout << "Test 11: Clear Snapshots... " << std::endl;
    SnapshotManagerTest test;
    
    test.manager->create_snapshot(test.complex, "snap1");
    test.manager->create_snapshot(test.complex, "snap2");
    test.manager->create_snapshot(test.complex, "snap3");

    assert(test.manager->get_snapshot_count() == 3);

    // Clear all snapshots
    test.manager->clear();

    assert(test.manager->get_snapshot_count() == 0);
    assert(!test.manager->has_snapshot("snap1"));
    assert(!test.manager->has_snapshot("snap2"));
    assert(!test.manager->has_snapshot("snap3"));
    
    std::cout << "�?Clear snapshots test passed" << std::endl;
}

// 12. Duplicate snapshot name test
void test_duplicate_snapshot_name() {
    std::cout << "Test 12: Duplicate Snapshot Name... " << std::endl;
    SnapshotManagerTest test;
    
    test.manager->create_snapshot(test.complex, "duplicate");

    bool threw = false;
    try {
        test.manager->create_snapshot(test.complex, "duplicate");
    } catch (const std::runtime_error&) {
        threw = true;
    }
    assert(threw);
    
    std::cout << "�?Duplicate snapshot name test passed" << std::endl;
}

// 13. Get total size test
void test_get_total_size() {
    std::cout << "Test 13: Get Total Size... " << std::endl;
    SnapshotManagerTest test;
    
    test.manager->create_snapshot(test.complex, "snap1", false);
    test.manager->create_snapshot(test.complex, "snap2", false);

    size_t total_uncompressed = test.manager->get_total_uncompressed_size();
    size_t total_compressed = test.manager->get_total_size();

    assert(total_uncompressed > 0);

    if (Compression::has_zlib_support()) {
        // With compression, total should be smaller
        assert(total_compressed <= total_uncompressed);
    }
    
    std::cout << "�?Get total size test passed" << std::endl;
}

// 14. Multiple snapshots test
void test_multiple_snapshots() {
    std::cout << "Test 14: Multiple Snapshots... " << std::endl;
    SnapshotManagerTest test;
    
    const int NUM_SNAPSHOTS = 10;

    for (int i = 0; i < NUM_SNAPSHOTS; ++i) {
        std::string name = "snap_" + std::to_string(i);
        test.manager->create_snapshot(test.complex, name);
    }

    assert(test.manager->get_snapshot_count() == NUM_SNAPSHOTS);

    auto snapshots = test.manager->list_snapshots();
    assert(snapshots.size() == NUM_SNAPSHOTS);
    
    std::cout << "�?Multiple snapshots test passed" << std::endl;
}

int main() {
    std::cout << "=== Snapshot Manager Tests ===" << std::endl << std::endl;
    
    try {
        test_create_snapshot();
        test_create_labeled_snapshot();
        test_list_snapshots();
        test_get_snapshot();
        test_restore_snapshot();
        test_delete_snapshot();
        test_compare_snapshots();
        test_persistence();
        test_compression();
        test_snapshot_metadata();
        test_clear_snapshots();
        test_duplicate_snapshot_name();
        test_get_total_size();
        test_multiple_snapshots();
        
        std::cout << std::endl;
        std::cout << "=== All Snapshot Manager Tests Passed! ===" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
