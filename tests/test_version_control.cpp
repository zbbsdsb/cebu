#include <cebu/version_control.h>
#include <gtest/gtest.h>

using namespace cebu;

class VersionControlTest : public ::testing::Test {
protected:
    void SetUp() override {
        vc = std::make_unique<VersionControl>("test_versions.ceb");
        build_test_complex();
    }

    void TearDown() override {
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

        complex.add_face(v0, v1, v2);
    }

    std::unique_ptr<VersionControl> vc;
    SimplicialComplex complex;
};

// 1. Commit version test
TEST_F(VersionControlTest, CommitVersion) {
    EXPECT_EQ(vc->get_version_count(), 0);

    VersionID v1 = vc->commit(complex, "Initial version", "alice");

    EXPECT_EQ(v1, 1);
    EXPECT_EQ(vc->get_version_count(), 1);
    EXPECT_EQ(vc->get_head(), v1);

    VersionID v2 = vc->commit(complex, "Second version", "bob");

    EXPECT_EQ(v2, 2);
    EXPECT_EQ(vc->get_version_count(), 2);
    EXPECT_EQ(vc->get_head(), v2);
}

// 2. Checkout version test
TEST_F(VersionControlTest, CheckoutVersion) {
    VersionID v1 = vc->commit(complex, "Version 1", "alice");

    // Modify complex
    VertexID v3 = complex.add_vertex();

    // Checkout version 1
    bool checked_out = vc->checkout(complex, v1);

    EXPECT_TRUE(checked_out);
    EXPECT_EQ(vc->get_head(), v1);
}

// 3. Log history test
TEST_F(VersionControlTest, LogHistory) {
    vc->commit(complex, "Commit 1", "alice");
    vc->commit(complex, "Commit 2", "bob");
    vc->commit(complex, "Commit 3", "alice");

    auto history = vc->log();

    EXPECT_EQ(history.size(), 3);

    // Check order (newest first)
    EXPECT_EQ(history[0].id, 3);
    EXPECT_EQ(history[1].id, 2);
    EXPECT_EQ(history[2].id, 1);

    // Limit log
    auto limited_history = vc->log(2);
    EXPECT_EQ(limited_history.size(), 2);
}

// 4. Version diff test
TEST_F(VersionControlTest, VersionDiff) {
    VersionID v1 = vc->commit(complex, "Version 1", "alice");

    // Modify complex
    VertexID v3 = complex.add_vertex();

    VersionID v2 = vc->commit(complex, "Version 2", "bob");

    // Diff
    auto changes = vc->diff(v1, v2);

    EXPECT_GE(changes.size(), 0);
}

// 5. Create branch test
TEST_F(VersionControlTest, CreateBranch) {
    VersionID v1 = vc->commit(complex, "Initial", "alice");

    std::string branch_name = vc->create_branch("feature", v1);

    EXPECT_EQ(branch_name, "feature");

    auto branches = vc->list_branches();
    EXPECT_EQ(branches.size(), 2); // main + feature

    // Check duplicate
    EXPECT_THROW(
        vc->create_branch("feature", v1),
        std::runtime_error
    );
}

// 6. Checkout branch test
TEST_F(VersionControlTest, CheckoutBranch) {
    vc->commit(complex, "Initial", "alice");
    vc->create_branch("feature");

    bool checked_out = vc->checkout_branch("feature");

    EXPECT_TRUE(checked_out);
    EXPECT_EQ(vc->get_current_branch(), "feature");

    // Checkout non-existent branch
    bool failed = vc->checkout_branch("non_existent");
    EXPECT_FALSE(failed);
}

// 7. Merge branch test
TEST_F(VersionControlTest, MergeBranch) {
    VersionID v1 = vc->commit(complex, "Initial", "alice");
    vc->create_branch("feature", v1);

    // Make changes on main
    VertexID v2 = complex.add_vertex();
    VersionID v2_id = vc->commit(complex, "Main change", "bob");

    // Checkout feature
    vc->checkout_branch("feature");

    // Merge
    bool merged = vc->merge_branch(complex, "main");

    EXPECT_TRUE(merged);
}

// 8. Delete branch test
TEST_F(VersionControlTest, DeleteBranch) {
    vc->commit(complex, "Initial", "alice");
    vc->create_branch("feature");

    EXPECT_EQ(vc->get_branch_count(), 2);

    bool deleted = vc->delete_branch("feature");

    EXPECT_TRUE(deleted);
    EXPECT_EQ(vc->get_branch_count(), 1);

    // Cannot delete current branch
    vc->checkout_branch("main");
    EXPECT_FALSE(vc->delete_branch("main"));

    // Cannot delete non-existent branch
    EXPECT_FALSE(vc->delete_branch("non_existent"));
}

// 9. Create tag test
TEST_F(VersionControlTest, CreateTag) {
    VersionID v1 = vc->commit(complex, "Version 1", "alice");

    std::string tag_name = vc->create_tag("v1.0.0", v1, "First release");

    EXPECT_EQ(tag_name, "v1.0.0");

    auto tags = vc->list_tags();
    EXPECT_EQ(tags.size(), 1);
    EXPECT_EQ(tags[0].name, "v1.0.0");

    // Duplicate tag
    EXPECT_THROW(
        vc->create_tag("v1.0.0", v1),
        std::runtime_error
    );
}

// 10. Delete tag test
TEST_F(VersionControlTest, DeleteTag) {
    VersionID v1 = vc->commit(complex, "Version 1", "alice");
    vc->create_tag("v1.0.0", v1);

    EXPECT_EQ(vc->get_tag_count(), 1);

    bool deleted = vc->delete_tag("v1.0.0");

    EXPECT_TRUE(deleted);
    EXPECT_EQ(vc->get_tag_count(), 0);

    // Delete non-existent tag
    EXPECT_FALSE(vc->delete_tag("non_existent"));
}

// 11. Revert version test
TEST_F(VersionControlTest, RevertVersion) {
    VersionID v1 = vc->commit(complex, "Version 1", "alice");

    // Modify complex
    VertexID v3 = complex.add_vertex();
    VersionID v2 = vc->commit(complex, "Version 2", "bob");

    size_t vertices_before = complex.vertex_count();

    // Revert without creating commit
    bool reverted = vc->revert(complex, v1, false);

    EXPECT_TRUE(reverted);
    EXPECT_EQ(vc->get_head(), v1);

    size_t vertices_after = complex.vertex_count();
    EXPECT_LT(vertices_after, vertices_before);
}

// 12. Revert with commit test
TEST_F(VersionControlTest, RevertWithCommit) {
    VersionID v1 = vc->commit(complex, "Version 1", "alice");
    VersionID v2 = vc->commit(complex, "Version 2", "bob");

    // Revert with commit
    bool reverted = vc->revert(complex, v1, true);

    EXPECT_TRUE(reverted);

    // Should create a new commit
    EXPECT_EQ(vc->get_version_count(), 3);
    EXPECT_GT(vc->get_head(), v2);
}

// 13. Get version test
TEST_F(VersionControlTest, GetVersion) {
    VersionID v1 = vc->commit(complex, "Version 1", "alice");

    auto version = vc->get_version(v1);

    EXPECT_EQ(version.vertex_count(), complex.vertex_count());
    EXPECT_EQ(version.simplex_count(), complex.simplex_count());

    // Get non-existent version
    EXPECT_THROW(
        vc->get_version(999),
        std::runtime_error
    );
}

// 14. Persistence test
TEST_F(VersionControlTest, Persistence) {
    vc->commit(complex, "Version 1", "alice");
    vc->commit(complex, "Version 2", "bob");
    vc->create_branch("feature");
    vc->create_tag("v1.0.0", 1);

    // Save
    bool saved = vc->save_to_file("test_versions.ceb");
    EXPECT_TRUE(saved);

    // Load
    VersionControl new_vc;
    bool loaded = new_vc.load_from_file("test_versions.ceb");
    EXPECT_TRUE(loaded);

    EXPECT_EQ(new_vc.get_version_count(), 2);
    EXPECT_EQ(new_vc.get_branch_count(), 2);
    EXPECT_EQ(new_vc.get_tag_count(), 1);
}

// 15. Head and branch queries test
TEST_F(VersionControlTest, HeadAndBranchQueries) {
    VersionID v1 = vc->commit(complex, "Version 1", "alice");

    EXPECT_EQ(vc->get_head(), v1);
    EXPECT_EQ(vc->get_current_branch(), "main");

    vc->create_branch("feature", v1);
    vc->checkout_branch("feature");

    EXPECT_EQ(vc->get_current_branch(), "feature");
}

// 16. Version metadata test
TEST_F(VersionControlTest, VersionMetadata) {
    VersionID v1 = vc->commit(complex, "Test message", "alice");

    auto history = vc->log(1);

    EXPECT_EQ(history[0].id, v1);
    EXPECT_EQ(history[0].message, "Test message");
    EXPECT_EQ(history[0].author, "alice");
    EXPECT_EQ(history[0].branch, "main");
    EXPECT_GT(history[0].simplex_count, 0);
}

// 17. Multiple versions test
TEST_F(VersionControlTest, MultipleVersions) {
    const int NUM_VERSIONS = 10;

    for (int i = 0; i < NUM_VERSIONS; ++i) {
        std::string message = "Commit " + std::to_string(i + 1);
        vc->commit(complex, message, "alice");
    }

    EXPECT_EQ(vc->get_version_count(), NUM_VERSIONS);

    auto history = vc->log();
    EXPECT_EQ(history.size(), NUM_VERSIONS);
}

// 18. Empty version control test
TEST_F(VersionControlTest, EmptyVersionControl) {
    EXPECT_EQ(vc->get_version_count(), 0);
    EXPECT_EQ(vc->get_branch_count(), 1); // main branch
    EXPECT_EQ(vc->get_tag_count(), 0);

    auto history = vc->log();
    EXPECT_TRUE(history.empty());

    auto branches = vc->list_branches();
    EXPECT_EQ(branches.size(), 1);
    EXPECT_EQ(branches[0].name, "main");

    auto tags = vc->list_tags();
    EXPECT_TRUE(tags.empty());
}

// 19. Labeled commit test
TEST_F(VersionControlTest, LabeledCommit) {
    SimplicialComplexLabeled<double> labeled;

    VertexID v0 = labeled.add_vertex();
    VertexID v1 = labeled.add_vertex();
    EdgeID e0 = labeled.add_edge(v0, v1);

    labeled.set_label(v0, 0.5);
    labeled.set_label(v1, 0.8);

    VersionID v1 = vc->commit_labeled(labeled, "Labeled version", "alice");

    EXPECT_EQ(v1, 1);
    EXPECT_EQ(vc->get_version_count(), 1);
}

// 20. Tag with description test
TEST_F(VersionControlTest, TagWithDescription) {
    VersionID v1 = vc->commit(complex, "Version 1", "alice");

    std::string desc = "First stable release";
    vc->create_tag("v1.0.0", v1, desc);

    auto tags = vc->list_tags();
    EXPECT_EQ(tags[0].description, desc);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
