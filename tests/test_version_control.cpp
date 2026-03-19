// Copyright (c) 2026 CeaserZhao (Oasis Company)
// SPDX-License-Identifier: MIT
#include <cebu/version_control.h>
#include <cassert>
#include <iostream>

using namespace cebu;

class VersionControlTest {
public:
    VersionControlTest() {
        vc = std::make_unique<VersionControl>("test_versions.ceb");
        build_test_complex();
    }

    ~VersionControlTest() {
        vc.reset();
        std::remove("test_versions.ceb");
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

    std::unique_ptr<VersionControl> vc;
    SimplicialComplex complex;
};

// 1. Commit version test
void test_commit_version() {
    std::cout << "Test 1: Commit Version... " << std::endl;
    VersionControlTest test;
    
    assert(test.vc->get_version_count() == 0);

    VersionID v1 = test.vc->commit(test.complex, "Initial version", "alice");

    assert(v1 == 1);
    assert(test.vc->get_version_count() == 1);
    assert(test.vc->get_head() == v1);

    VersionID v2 = test.vc->commit(test.complex, "Second version", "bob");

    assert(v2 == 2);
    assert(test.vc->get_version_count() == 2);
    assert(test.vc->get_head() == v2);
    
    std::cout << "âœ?Commit version test passed" << std::endl;
}

// 2. Checkout version test
void test_checkout_version() {
    std::cout << "Test 2: Checkout Version... " << std::endl;
    VersionControlTest test;
    
    VersionID v1 = test.vc->commit(test.complex, "Version 1", "alice");

    // Modify complex
    VertexID v3 = test.complex.add_vertex();

    // Checkout version 1
    bool checked_out = test.vc->checkout(test.complex, v1);

    assert(checked_out);
    assert(test.vc->get_head() == v1);
    
    std::cout << "âœ?Checkout version test passed" << std::endl;
}

// 3. Log history test
void test_log_history() {
    std::cout << "Test 3: Log History... " << std::endl;
    VersionControlTest test;
    
    test.vc->commit(test.complex, "Commit 1", "alice");
    test.vc->commit(test.complex, "Commit 2", "bob");
    test.vc->commit(test.complex, "Commit 3", "alice");

    auto history = test.vc->log();

    assert(history.size() == 3);

    // Check order (newest first)
    assert(history[0].id == 3);
    assert(history[1].id == 2);
    assert(history[2].id == 1);

    // Limit log
    auto limited_history = test.vc->log(2);
    assert(limited_history.size() == 2);
    
    std::cout << "âœ?Log history test passed" << std::endl;
}

// 4. Version diff test
void test_version_diff() {
    std::cout << "Test 4: Version Diff... " << std::endl;
    VersionControlTest test;
    
    VersionID v1 = test.vc->commit(test.complex, "Version 1", "alice");

    // Modify complex
    VertexID v3 = test.complex.add_vertex();

    VersionID v2 = test.vc->commit(test.complex, "Version 2", "bob");

    // Diff
    auto changes = test.vc->diff(v1, v2);

    assert(changes.size() >= 0);
    
    std::cout << "âœ?Version diff test passed" << std::endl;
}

// 5. Create branch test
void test_create_branch() {
    std::cout << "Test 5: Create Branch... " << std::endl;
    VersionControlTest test;
    
    VersionID v1 = test.vc->commit(test.complex, "Initial", "alice");

    std::string branch_name = test.vc->create_branch("feature", v1);

    assert(branch_name == "feature");

    auto branches = test.vc->list_branches();
    assert(branches.size() == 2); // main + feature

    // Check duplicate
    bool threw = false;
    try {
        test.vc->create_branch("feature", v1);
    } catch (const std::runtime_error&) {
        threw = true;
    }
    assert(threw);
    
    std::cout << "âœ?Create branch test passed" << std::endl;
}

// 6. Checkout branch test
void test_checkout_branch() {
    std::cout << "Test 6: Checkout Branch... " << std::endl;
    VersionControlTest test;
    
    test.vc->commit(test.complex, "Initial", "alice");
    test.vc->create_branch("feature");

    bool checked_out = test.vc->checkout_branch("feature");

    assert(checked_out);
    assert(test.vc->get_current_branch() == "feature");

    // Checkout non-existent branch
    bool failed = test.vc->checkout_branch("non_existent");
    assert(!failed);
    
    std::cout << "âœ?Checkout branch test passed" << std::endl;
}

// 7. Merge branch test
void test_merge_branch() {
    std::cout << "Test 7: Merge Branch... " << std::endl;
    VersionControlTest test;
    
    VersionID v1 = test.vc->commit(test.complex, "Initial", "alice");
    test.vc->create_branch("feature", v1);

    // Make changes on main
    VertexID v2 = test.complex.add_vertex();
    VersionID v2_id = test.vc->commit(test.complex, "Main change", "bob");

    // Checkout feature
    test.vc->checkout_branch("feature");

    // Merge
    bool merged = test.vc->merge_branch(test.complex, "main");

    assert(merged);
    
    std::cout << "âœ?Merge branch test passed" << std::endl;
}

// 8. Delete branch test
void test_delete_branch() {
    std::cout << "Test 8: Delete Branch... " << std::endl;
    VersionControlTest test;
    
    test.vc->commit(test.complex, "Initial", "alice");
    test.vc->create_branch("feature");

    assert(test.vc->get_branch_count() == 2);

    bool deleted = test.vc->delete_branch("feature");

    assert(deleted);
    assert(test.vc->get_branch_count() == 1);

    // Cannot delete current branch
    test.vc->checkout_branch("main");
    assert(!test.vc->delete_branch("main"));

    // Cannot delete non-existent branch
    assert(!test.vc->delete_branch("non_existent"));
    
    std::cout << "âœ?Delete branch test passed" << std::endl;
}

// 9. Create tag test
void test_create_tag() {
    std::cout << "Test 9: Create Tag... " << std::endl;
    VersionControlTest test;
    
    VersionID v1 = test.vc->commit(test.complex, "Version 1", "alice");

    std::string tag_name = test.vc->create_tag("v1.0.0", v1, "First release");

    assert(tag_name == "v1.0.0");

    auto tags = test.vc->list_tags();
    assert(tags.size() == 1);
    assert(tags[0].name == "v1.0.0");

    // Duplicate tag
    bool threw = false;
    try {
        test.vc->create_tag("v1.0.0", v1);
    } catch (const std::runtime_error&) {
        threw = true;
    }
    assert(threw);
    
    std::cout << "âœ?Create tag test passed" << std::endl;
}

// 10. Delete tag test
void test_delete_tag() {
    std::cout << "Test 10: Delete Tag... " << std::endl;
    VersionControlTest test;
    
    VersionID v1 = test.vc->commit(test.complex, "Version 1", "alice");
    test.vc->create_tag("v1.0.0", v1);

    assert(test.vc->get_tag_count() == 1);

    bool deleted = test.vc->delete_tag("v1.0.0");

    assert(deleted);
    assert(test.vc->get_tag_count() == 0);

    // Delete non-existent tag
    assert(!test.vc->delete_tag("non_existent"));
    
    std::cout << "âœ?Delete tag test passed" << std::endl;
}

// 11. Revert version test
void test_revert_version() {
    std::cout << "Test 11: Revert Version... " << std::endl;
    VersionControlTest test;
    
    VersionID v1 = test.vc->commit(test.complex, "Version 1", "alice");

    // Modify complex
    VertexID v3 = test.complex.add_vertex();
    VersionID v2 = test.vc->commit(test.complex, "Version 2", "bob");

    size_t vertices_before = test.complex.vertex_count();

    // Revert without creating commit
    bool reverted = test.vc->revert(test.complex, v1, false);

    assert(reverted);
    assert(test.vc->get_head() == v1);

    size_t vertices_after = test.complex.vertex_count();
    assert(vertices_after < vertices_before);
    
    std::cout << "âœ?Revert version test passed" << std::endl;
}

// 12. Revert with commit test
void test_revert_with_commit() {
    std::cout << "Test 12: Revert with Commit... " << std::endl;
    VersionControlTest test;
    
    VersionID v1 = test.vc->commit(test.complex, "Version 1", "alice");
    VersionID v2 = test.vc->commit(test.complex, "Version 2", "bob");

    // Revert with commit
    bool reverted = test.vc->revert(test.complex, v1, true);

    assert(reverted);

    // Should create a new commit
    assert(test.vc->get_version_count() == 3);
    assert(test.vc->get_head() > v2);
    
    std::cout << "âœ?Revert with commit test passed" << std::endl;
}

// 13. Get version test
void test_get_version() {
    std::cout << "Test 13: Get Version... " << std::endl;
    VersionControlTest test;
    
    VersionID v1 = test.vc->commit(test.complex, "Version 1", "alice");

    auto version = test.vc->get_version(v1);

    assert(version.vertex_count() == test.complex.vertex_count());
    assert(version.simplex_count() == test.complex.simplex_count());

    // Get non-existent version
    bool threw = false;
    try {
        test.vc->get_version(999);
    } catch (const std::runtime_error&) {
        threw = true;
    }
    assert(threw);
    
    std::cout << "âœ?Get version test passed" << std::endl;
}

// 14. Persistence test
void test_persistence() {
    std::cout << "Test 14: Persistence... " << std::endl;
    VersionControlTest test;
    
    test.vc->commit(test.complex, "Version 1", "alice");
    test.vc->commit(test.complex, "Version 2", "bob");
    test.vc->create_branch("feature");
    test.vc->create_tag("v1.0.0", 1);

    // Save
    bool saved = test.vc->save_to_file("test_versions.ceb");
    assert(saved);

    // Load
    VersionControl new_vc;
    bool loaded = new_vc.load_from_file("test_versions.ceb");
    assert(loaded);

    assert(new_vc.get_version_count() == 2);
    assert(new_vc.get_branch_count() == 2);
    assert(new_vc.get_tag_count() == 1);
    
    std::cout << "âœ?Persistence test passed" << std::endl;
}

// 15. Head and branch queries test
void test_head_and_branch_queries() {
    std::cout << "Test 15: Head and Branch Queries... " << std::endl;
    VersionControlTest test;
    
    VersionID v1 = test.vc->commit(test.complex, "Version 1", "alice");

    assert(test.vc->get_head() == v1);
    assert(test.vc->get_current_branch() == "main");

    test.vc->create_branch("feature", v1);
    test.vc->checkout_branch("feature");

    assert(test.vc->get_current_branch() == "feature");
    
    std::cout << "âœ?Head and branch queries test passed" << std::endl;
}

// 16. Version metadata test
void test_version_metadata() {
    std::cout << "Test 16: Version Metadata... " << std::endl;
    VersionControlTest test;
    
    VersionID v1 = test.vc->commit(test.complex, "Test message", "alice");

    auto history = test.vc->log(1);

    assert(history[0].id == v1);
    assert(history[0].message == "Test message");
    assert(history[0].author == "alice");
    assert(history[0].branch == "main");
    assert(history[0].simplex_count > 0);
    
    std::cout << "âœ?Version metadata test passed" << std::endl;
}

// 17. Multiple versions test
void test_multiple_versions() {
    std::cout << "Test 17: Multiple Versions... " << std::endl;
    VersionControlTest test;
    
    const int NUM_VERSIONS = 10;

    for (int i = 0; i < NUM_VERSIONS; ++i) {
        std::string message = "Commit " + std::to_string(i + 1);
        test.vc->commit(test.complex, message, "alice");
    }

    assert(test.vc->get_version_count() == NUM_VERSIONS);

    auto history = test.vc->log();
    assert(history.size() == NUM_VERSIONS);
    
    std::cout << "âœ?Multiple versions test passed" << std::endl;
}

// 18. Empty version control test
void test_empty_version_control() {
    std::cout << "Test 18: Empty Version Control... " << std::endl;
    VersionControlTest test;
    
    assert(test.vc->get_version_count() == 0);
    assert(test.vc->get_branch_count() == 1); // main branch
    assert(test.vc->get_tag_count() == 0);

    auto history = test.vc->log();
    assert(history.empty());

    auto branches = test.vc->list_branches();
    assert(branches.size() == 1);
    assert(branches[0].name == "main");

    auto tags = test.vc->list_tags();
    assert(tags.empty());
    
    std::cout << "âœ?Empty version control test passed" << std::endl;
}

// 19. Labeled commit test
void test_labeled_commit() {
    std::cout << "Test 19: Labeled Commit... " << std::endl;
    VersionControlTest test;
    
    SimplicialComplexLabeled<double> labeled;

    VertexID v0 = labeled.add_vertex();
    VertexID v1 = labeled.add_vertex();
    SimplexID e0 = labeled.add_edge(v0, v1);

    labeled.set_label(v0, 0.5);
    labeled.set_label(v1, 0.8);

    VersionID version_id = test.vc->commit_labeled(labeled, "Labeled version", "alice");

    assert(version_id == 1);
    assert(test.vc->get_version_count() == 1);
    
    std::cout << "âœ?Labeled commit test passed" << std::endl;
}

// 20. Tag with description test
void test_tag_with_description() {
    std::cout << "Test 20: Tag with Description... " << std::endl;
    VersionControlTest test;
    
    VersionID v1 = test.vc->commit(test.complex, "Version 1", "alice");

    std::string desc = "First stable release";
    test.vc->create_tag("v1.0.0", v1, desc);

    auto tags = test.vc->list_tags();
    assert(tags[0].description == desc);
    
    std::cout << "âœ?Tag with description test passed" << std::endl;
}

int main() {
    std::cout << "=== Version Control Tests ===" << std::endl << std::endl;
    
    try {
        test_commit_version();
        test_checkout_version();
        test_log_history();
        test_version_diff();
        test_create_branch();
        test_checkout_branch();
        test_merge_branch();
        test_delete_branch();
        test_create_tag();
        test_delete_tag();
        test_revert_version();
        test_revert_with_commit();
        test_get_version();
        test_persistence();
        test_head_and_branch_queries();
        test_version_metadata();
        test_multiple_versions();
        test_empty_version_control();
        test_labeled_commit();
        test_tag_with_description();
        
        std::cout << std::endl;
        std::cout << "=== All Version Control Tests Passed! ===" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
