#include <cebu/change_tracker.h>
#include <nlohmann/json.hpp>
#include <thread>
#include <chrono>
#include <cassert>
#include <iostream>

using namespace cebu;

void test_basic_tracking() {
    std::cout << "Testing basic tracking..." << std::endl;
    
    auto tracker = std::make_unique<ChangeTracker>();
    
    assert(!tracker->has_changes());
    assert(tracker->get_change_count() == 0);

    // Track simplex added
    tracker->track_simplex_added(1, 0, {0});
    tracker->track_simplex_added(2, 1, {0, 1});

    // Track simplex removed
    tracker->track_simplex_removed(3, 0, {2});

    assert(tracker->has_changes());
    assert(tracker->get_change_count() == 3);

    // Track label changed
    tracker->track_label_changed(1, 0.0, 0.5);

    assert(tracker->get_change_count() == 4);

    // Track equivalence changed
    tracker->track_equivalence_changed(1, 0, 1);

    assert(tracker->get_change_count() == 5);

    // Track topology changed
    tracker->track_topology_changed();

    assert(tracker->get_change_count() == 6);
    
    std::cout << "  OK: Basic tracking works" << std::endl;
}

// 2. Query by type test
void test_query_by_type() {
    std::cout << "Testing query by type..." << std::endl;
    
    auto tracker = std::make_unique<ChangeTracker>();
    
    tracker->track_simplex_added(1, 0, {0});
    tracker->track_simplex_added(2, 1, {0, 1});
    tracker->track_simplex_removed(3, 0, {2});
    tracker->track_label_changed(1, 0.0, 0.5);
    tracker->track_equivalence_changed(1, 0, 1);
    tracker->track_topology_changed();

    // Query added simplices
    auto added = tracker->get_changes_by_type(ChangeType::SIMPLEX_ADDED);
    assert(added.size() == 2);

    // Query removed simplices
    auto removed = tracker->get_changes_by_type(ChangeType::SIMPLEX_REMOVED);
    assert(removed.size() == 1);

    // Query label changes
    auto label_changes = tracker->get_changes_by_type(ChangeType::LABEL_CHANGED);
    assert(label_changes.size() == 1);
    assert(std::abs(label_changes[0].old_label - 0.0) < 1e-6);
    assert(std::abs(label_changes[0].new_label - 0.5) < 1e-6);

    // Query equivalence changes
    auto eq_changes = tracker->get_changes_by_type(ChangeType::EQUIVALENCE_CHANGED);
    assert(eq_changes.size() == 1);
    assert(eq_changes[0].old_equivalence == 0);
    assert(eq_changes[0].new_equivalence == 1);

    // Query topology changes
    auto topo_changes = tracker->get_changes_by_type(ChangeType::TOPOLOGY_CHANGED);
    assert(topo_changes.size() == 1);
    
    std::cout << "  OK: Query by type works" << std::endl;
}

// 3. Query by simplex test
void test_query_by_simplex() {
    std::cout << "Testing query by simplex..." << std::endl;
    
    auto tracker = std::make_unique<ChangeTracker>();
    
    tracker->track_simplex_added(1, 0, {0});
    tracker->track_simplex_added(2, 1, {0, 1});
    tracker->track_label_changed(1, 0.0, 0.5);
    tracker->track_label_changed(2, 0.0, 0.8);
    tracker->track_simplex_removed(1, 0, {0});

    // Query changes for simplex 1
    auto changes_1 = tracker->get_changes_by_simplex(1);
    assert(changes_1.size() == 3);

    // Query changes for simplex 2
    auto changes_2 = tracker->get_changes_by_simplex(2);
    assert(changes_2.size() == 2);

    // Query changes for non-existent simplex
    auto changes_99 = tracker->get_changes_by_simplex(99);
    assert(changes_99.size() == 0);
    
    std::cout << "  OK: Query by simplex works" << std::endl;
}

// 4. Time range query test
void test_time_range_query() {
    std::cout << "Testing time range query..." << std::endl;
    
    auto tracker = std::make_unique<ChangeTracker>();
    
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
    assert(all_changes.size() == 5);

    // Query in time range [mid, last]
    auto range_changes = tracker->get_changes_in_time_range(mid_time, last_time);
    assert(range_changes.size() >= 2);

    // Query before first change
    auto before_changes = tracker->get_changes_in_time_range(
        std::chrono::system_clock::now() - std::chrono::hours(1), first_time);
    assert(before_changes.size() >= 1);
    
    std::cout << "  OK: Time range query works" << std::endl;
}

// 5. JSON export/import test
void test_json_export_import() {
    std::cout << "Testing JSON export/import..." << std::endl;
    
    auto tracker = std::make_unique<ChangeTracker>();
    
    tracker->track_simplex_added(1, 0, {0});
    tracker->track_simplex_added(2, 1, {0, 1});
    tracker->track_label_changed(1, 0.0, 0.5);
    tracker->track_simplex_removed(1, 0, {0});

    // Export to JSON
    nlohmann::json j = tracker->to_json();
    assert(j.contains("format"));
    assert(j.contains("version"));
    assert(j.contains("changes"));

    assert(j["format"].get<std::string>() == "cebu_delta");
    assert(j["change_count"].get<size_t>() == 4);

    // Import from JSON
    ChangeTracker new_tracker;
    new_tracker.from_json(j);

    assert(new_tracker.get_change_count() == tracker->get_change_count());
    
    std::cout << "  OK: JSON export/import works" << std::endl;
}

// 6. File save/load test
void test_file_save_load() {
    std::cout << "Testing file save/load..." << std::endl;
    
    auto tracker = std::make_unique<ChangeTracker>();
    
    std::string filename = "test_delta.json";

    tracker->track_simplex_added(1, 0, {0});
    tracker->track_simplex_added(2, 1, {0, 1});
    tracker->track_label_changed(1, 0.0, 0.5);
    tracker->track_equivalence_changed(1, 0, 1);

    // Save to file
    bool saved = tracker->save_to_file(filename);
    assert(saved);

    // Load from file
    ChangeTracker new_tracker;
    bool loaded = new_tracker.load_from_file(filename);
    assert(loaded);

    assert(new_tracker.get_change_count() == tracker->get_change_count());

    // Clean up
    std::remove(filename.c_str());
    
    std::cout << "  OK: File save/load works" << std::endl;
}

// 7. Reset test
void test_reset() {
    std::cout << "Testing reset..." << std::endl;
    
    auto tracker = std::make_unique<ChangeTracker>();
    
    tracker->track_simplex_added(1, 0, {0});
    tracker->track_simplex_added(2, 1, {0, 1});
    tracker->track_label_changed(1, 0.0, 0.5);

    assert(tracker->get_change_count() == 3);
    assert(tracker->has_changes());

    // Reset
    tracker->reset();

    assert(tracker->get_change_count() == 0);
    assert(!tracker->has_changes());
    
    std::cout << "  OK: Reset works" << std::endl;
}

// 8. Multiple changes test
void test_multiple_changes() {
    std::cout << "Testing multiple changes..." << std::endl;
    
    auto tracker = std::make_unique<ChangeTracker>();
    
    const int NUM_CHANGES = 1000;

    for (int i = 0; i < NUM_CHANGES; ++i) {
        tracker->track_simplex_added(static_cast<SimplexID>(i), 0, {static_cast<VertexID>(i)});
    }

    assert(tracker->get_change_count() == NUM_CHANGES);

    auto added = tracker->get_changes_by_type(ChangeType::SIMPLEX_ADDED);
    assert(added.size() == NUM_CHANGES);
    
    std::cout << "  OK: Multiple changes works" << std::endl;
}

// 9. Empty tracker test
void test_empty_tracker() {
    std::cout << "Testing empty tracker..." << std::endl;
    
    auto tracker = std::make_unique<ChangeTracker>();
    
    assert(!tracker->has_changes());
    assert(tracker->get_change_count() == 0);

    auto changes = tracker->get_changes();
    assert(changes.empty());

    auto added = tracker->get_changes_by_type(ChangeType::SIMPLEX_ADDED);
    assert(added.empty());

    auto by_simplex = tracker->get_changes_by_simplex(1);
    assert(by_simplex.empty());

    // JSON export should still work
    nlohmann::json j = tracker->to_json();
    assert(j.contains("changes"));
    assert(j["changes"].size() == 0);
    
    std::cout << "  OK: Empty tracker works" << std::endl;
}

// 10. JSON validation test
void test_json_validation() {
    std::cout << "Testing JSON validation..." << std::endl;
    
    auto tracker = std::make_unique<ChangeTracker>();
    
    tracker->track_simplex_added(1, 0, {0});
    tracker->track_label_changed(1, 0.0, 0.5);

    nlohmann::json j = tracker->to_json();

    // Valid JSON
    ChangeTracker valid_tracker;
    try {
        valid_tracker.from_json(j);
    } catch (const std::exception& e) {
        assert(false && "Valid JSON should not throw");
    }

    // Invalid format
    nlohmann::json invalid;
    invalid["format"] = "invalid_format";
    invalid["changes"] = nlohmann::json::array();

    ChangeTracker invalid_tracker;
    bool threw = false;
    try {
        invalid_tracker.from_json(invalid);
    } catch (const std::runtime_error&) {
        threw = true;
    }
    assert(threw);

    // Missing changes field
    nlohmann::json missing_changes;
    missing_changes["format"] = "cebu_delta";

    ChangeTracker missing_tracker;
    threw = false;
    try {
        missing_tracker.from_json(missing_changes);
    } catch (const std::runtime_error&) {
        threw = true;
    }
    assert(threw);
    
    std::cout << "  OK: JSON validation works" << std::endl;
}

int main() {
    std::cout << "=== Change Tracker Tests ===" << std::endl;
    
    try {
        test_basic_tracking();
        test_query_by_type();
        test_query_by_simplex();
        test_time_range_query();
        test_json_export_import();
        test_file_save_load();
        test_reset();
        test_multiple_changes();
        test_empty_tracker();
        test_json_validation();
        
        std::cout << "\nAll change tracker tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
