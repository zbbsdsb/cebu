#include <cebu/change_tracker.h>
#include <gtest/gtest.h>
#include <thread>
#include <chrono>

using namespace cebu;

class ChangeTrackerTest : public ::testing::Test {
protected:
    void SetUp() override {
        tracker = std::make_unique<ChangeTracker>();
    }

    void TearDown() override {
        tracker.reset();
    }

    std::unique_ptr<ChangeTracker> tracker;
};

// 1. Basic tracking test
TEST_F(ChangeTrackerTest, BasicTracking) {
    EXPECT_FALSE(tracker->has_changes());
    EXPECT_EQ(tracker->get_change_count(), 0);

    // Track simplex added
    tracker->track_simplex_added(1, 0, {0});
    tracker->track_simplex_added(2, 1, {0, 1});

    // Track simplex removed
    tracker->track_simplex_removed(3, 0, {2});

    EXPECT_TRUE(tracker->has_changes());
    EXPECT_EQ(tracker->get_change_count(), 3);

    // Track label changed
    tracker->track_label_changed(1, 0.0, 0.5);

    EXPECT_EQ(tracker->get_change_count(), 4);

    // Track equivalence changed
    tracker->track_equivalence_changed(1, 0, 1);

    EXPECT_EQ(tracker->get_change_count(), 5);

    // Track topology changed
    tracker->track_topology_changed();

    EXPECT_EQ(tracker->get_change_count(), 6);
}

// 2. Query by type test
TEST_F(ChangeTrackerTest, QueryByType) {
    tracker->track_simplex_added(1, 0, {0});
    tracker->track_simplex_added(2, 1, {0, 1});
    tracker->track_simplex_removed(3, 0, {2});
    tracker->track_label_changed(1, 0.0, 0.5);
    tracker->track_equivalence_changed(1, 0, 1);
    tracker->track_topology_changed();

    // Query added simplices
    auto added = tracker->get_changes_by_type(ChangeType::SIMPLEX_ADDED);
    EXPECT_EQ(added.size(), 2);

    // Query removed simplices
    auto removed = tracker->get_changes_by_type(ChangeType::SIMPLEX_REMOVED);
    EXPECT_EQ(removed.size(), 1);

    // Query label changes
    auto label_changes = tracker->get_changes_by_type(ChangeType::LABEL_CHANGED);
    EXPECT_EQ(label_changes.size(), 1);
    EXPECT_DOUBLE_EQ(label_changes[0].old_label, 0.0);
    EXPECT_DOUBLE_EQ(label_changes[0].new_label, 0.5);

    // Query equivalence changes
    auto eq_changes = tracker->get_changes_by_type(ChangeType::EQUIVALENCE_CHANGED);
    EXPECT_EQ(eq_changes.size(), 1);
    EXPECT_EQ(eq_changes[0].old_equivalence, 0);
    EXPECT_EQ(eq_changes[0].new_equivalence, 1);

    // Query topology changes
    auto topo_changes = tracker->get_changes_by_type(ChangeType::TOPOLOGY_CHANGED);
    EXPECT_EQ(topo_changes.size(), 1);
}

// 3. Query by simplex test
TEST_F(ChangeTrackerTest, QueryBySimplex) {
    tracker->track_simplex_added(1, 0, {0});
    tracker->track_simplex_added(2, 1, {0, 1});
    tracker->track_label_changed(1, 0.0, 0.5);
    tracker->track_label_changed(2, 0.0, 0.8);
    tracker->track_simplex_removed(1, 0, {0});

    // Query changes for simplex 1
    auto changes_1 = tracker->get_changes_by_simplex(1);
    EXPECT_EQ(changes_1.size(), 3);

    // Query changes for simplex 2
    auto changes_2 = tracker->get_changes_by_simplex(2);
    EXPECT_EQ(changes_2.size(), 2);

    // Query changes for non-existent simplex
    auto changes_99 = tracker->get_changes_by_simplex(99);
    EXPECT_EQ(changes_99.size(), 0);
}

// 4. Time range query test
TEST_F(ChangeTrackerTest, TimeRangeQuery) {
    // Track first change
    tracker->track_simplex_added(1, 0, {0});
    auto first_time = std::chrono::system_clock::now();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // Track more changes
    tracker->track_simplex_added(2, 1, {0, 1});
    tracker->track_label_changed(1, 0.0, 0.5);

    auto mid_time = std::chrono::system_clock::now();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // Track final changes
    tracker->track_simplex_removed(2, 1, {0, 1});
    tracker->track_topology_changed();

    auto last_time = std::chrono::system_clock::now();

    // Query all changes
    auto all_changes = tracker->get_changes();
    EXPECT_EQ(all_changes.size(), 5);

    // Query in time range [mid, last]
    auto range_changes = tracker->get_changes_in_time_range(mid_time, last_time);
    EXPECT_GE(range_changes.size(), 2);

    // Query before first change
    auto before_changes = tracker->get_changes_in_time_range(
        std::chrono::system_clock::now() - std::chrono::hours(1), first_time);
    EXPECT_GE(before_changes.size(), 1);
}

// 5. JSON export/import test
TEST_F(ChangeTrackerTest, JSONExportImport) {
    tracker->track_simplex_added(1, 0, {0});
    tracker->track_simplex_added(2, 1, {0, 1});
    tracker->track_label_changed(1, 0.0, 0.5);
    tracker->track_simplex_removed(1, 0, {0});

    // Export to JSON
    nlohmann::json j = tracker->to_json();
    EXPECT_TRUE(j.contains("format"));
    EXPECT_TRUE(j.contains("version"));
    EXPECT_TRUE(j.contains("changes"));

    EXPECT_EQ(j["format"].get<std::string>(), "cebu_delta");
    EXPECT_EQ(j["change_count"].get<size_t>(), 4);

    // Import from JSON
    ChangeTracker new_tracker;
    new_tracker.from_json(j);

    EXPECT_EQ(new_tracker.get_change_count(), tracker->get_change_count());
}

// 6. File save/load test
TEST_F(ChangeTrackerTest, FileSaveLoad) {
    std::string filename = "test_delta.json";

    tracker->track_simplex_added(1, 0, {0});
    tracker->track_simplex_added(2, 1, {0, 1});
    tracker->track_label_changed(1, 0.0, 0.5);
    tracker->track_equivalence_changed(1, 0, 1);

    // Save to file
    bool saved = tracker->save_to_file(filename);
    EXPECT_TRUE(saved);

    // Load from file
    ChangeTracker new_tracker;
    bool loaded = new_tracker.load_from_file(filename);
    EXPECT_TRUE(loaded);

    EXPECT_EQ(new_tracker.get_change_count(), tracker->get_change_count());

    // Clean up
    std::remove(filename.c_str());
}

// 7. Reset test
TEST_F(ChangeTrackerTest, Reset) {
    tracker->track_simplex_added(1, 0, {0});
    tracker->track_simplex_added(2, 1, {0, 1});
    tracker->track_label_changed(1, 0.0, 0.5);

    EXPECT_EQ(tracker->get_change_count(), 3);
    EXPECT_TRUE(tracker->has_changes());

    // Reset
    tracker->reset();

    EXPECT_EQ(tracker->get_change_count(), 0);
    EXPECT_FALSE(tracker->has_changes());
}

// 8. Multiple changes test
TEST_F(ChangeTrackerTest, MultipleChanges) {
    const int NUM_CHANGES = 1000;

    for (int i = 0; i < NUM_CHANGES; ++i) {
        tracker->track_simplex_added(i, 0, {i});
    }

    EXPECT_EQ(tracker->get_change_count(), NUM_CHANGES);

    auto added = tracker->get_changes_by_type(ChangeType::SIMPLEX_ADDED);
    EXPECT_EQ(added.size(), NUM_CHANGES);
}

// 9. Empty tracker test
TEST_F(ChangeTrackerTest, EmptyTracker) {
    EXPECT_FALSE(tracker->has_changes());
    EXPECT_EQ(tracker->get_change_count(), 0);

    auto changes = tracker->get_changes();
    EXPECT_TRUE(changes.empty());

    auto added = tracker->get_changes_by_type(ChangeType::SIMPLEX_ADDED);
    EXPECT_TRUE(added.empty());

    auto by_simplex = tracker->get_changes_by_simplex(1);
    EXPECT_TRUE(by_simplex.empty());

    // JSON export should still work
    nlohmann::json j = tracker->to_json();
    EXPECT_TRUE(j.contains("changes"));
    EXPECT_EQ(j["changes"].size(), 0);
}

// 10. JSON validation test
TEST_F(ChangeTrackerTest, JSONValidation) {
    tracker->track_simplex_added(1, 0, {0});
    tracker->track_label_changed(1, 0.0, 0.5);

    nlohmann::json j = tracker->to_json();

    // Valid JSON
    ChangeTracker valid_tracker;
    EXPECT_NO_THROW(valid_tracker.from_json(j));

    // Invalid format
    nlohmann::json invalid;
    invalid["format"] = "invalid_format";
    invalid["changes"] = nlohmann::json::array();

    ChangeTracker invalid_tracker;
    EXPECT_THROW(invalid_tracker.from_json(invalid), std::runtime_error);

    // Missing changes field
    nlohmann::json missing_changes;
    missing_changes["format"] = "cebu_delta";

    ChangeTracker missing_tracker;
    EXPECT_THROW(missing_tracker.from_json(missing_changes), std::runtime_error);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
