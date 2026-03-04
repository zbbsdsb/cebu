#include "cebu/timeline.h"
#include <iostream>
#include <cassert>
#include <utility>

using namespace cebu;

void test_basic_timeline() {
    std::cout << "Testing basic timeline construction..." << std::endl;

    Timeline timeline(0.0, 10.0);

    auto bounds = timeline.get_bounds();
    assert(bounds.first == 0.0);
    assert(bounds.second == 10.0);
    assert(timeline.duration() == 10.0);

    std::cout << "  OK: Basic timeline works" << std::endl;
}

void test_contains_timestamp() {
    std::cout << "Testing timestamp containment..." << std::endl;

    Timeline timeline(5.0, 15.0);

    assert(timeline.contains(5.0));
    assert(timeline.contains(10.0));
    assert(timeline.contains(15.0));
    assert(!timeline.contains(4.9));
    assert(!timeline.contains(15.1));

    std::cout << "  OK: Timestamp containment works" << std::endl;
}

void test_set_bounds() {
    std::cout << "Testing set bounds..." << std::endl;

    Timeline timeline;
    assert(timeline.duration() == 1.0);  // Default 0.0 to 1.0

    timeline.set_bounds(2.0, 8.0);
    assert(timeline.get_bounds().first == 2.0);
    assert(timeline.get_bounds().second == 8.0);
    assert(timeline.duration() == 6.0);

    std::cout << "  OK: Set bounds works" << std::endl;
}

void test_add_milestone() {
    std::cout << "Testing add milestone..." << std::endl;

    Timeline timeline(0.0, 10.0);

    timeline.add_milestone(2.5, "Start");
    timeline.add_milestone(5.0, "Middle");
    timeline.add_milestone(7.5, "End");

    assert(timeline.milestone_count() == 3);

    auto milestones = timeline.get_milestones();
    assert(milestones[0].first == 2.5);
    assert(milestones[0].second == "Start");
    assert(milestones[1].first == 5.0);
    assert(milestones[2].first == 7.5);

    std::cout << "  OK: Add milestone works" << std::endl;
}

void test_milestone_ordering() {
    std::cout << "Testing milestone ordering..." << std::endl;

    Timeline timeline(0.0, 10.0);

    // Add in random order
    timeline.add_milestone(5.0, "Second");
    timeline.add_milestone(1.0, "First");
    timeline.add_milestone(9.0, "Last");

    auto milestones = timeline.get_milestones();
    assert(milestones[0].first == 1.0);
    assert(milestones[1].first == 5.0);
    assert(milestones[2].first == 9.0);

    std::cout << "  OK: Milestone ordering works" << std::endl;
}

void test_update_milestone() {
    std::cout << "Testing update milestone..." << std::endl;

    Timeline timeline(0.0, 10.0);

    timeline.add_milestone(3.0, "Original");
    timeline.add_milestone(3.0, "Updated");

    assert(timeline.milestone_count() == 1);
    assert(*timeline.get_milestone_at(3.0) == "Updated");

    std::cout << "  OK: Update milestone works" << std::endl;
}

void test_get_milestones_in_range() {
    std::cout << "Testing get milestones in range..." << std::endl;

    Timeline timeline(0.0, 10.0);

    timeline.add_milestone(1.0, "A");
    timeline.add_milestone(3.0, "B");
    timeline.add_milestone(5.0, "C");
    timeline.add_milestone(7.0, "D");
    timeline.add_milestone(9.0, "E");

    auto range = timeline.get_milestones_in_range(2.0, 7.0);
    assert(range.size() == 3);
    assert(range[0].second == "B");
    assert(range[1].second == "C");
    assert(range[2].second == "D");

    std::cout << "  OK: Get milestones in range works" << std::endl;
}

void test_remove_milestone() {
    std::cout << "Testing remove milestone..." << std::endl;

    Timeline timeline(0.0, 10.0);

    timeline.add_milestone(2.0, "A");
    timeline.add_milestone(5.0, "B");
    timeline.add_milestone(7.0, "C");

    assert(timeline.remove_milestone(5.0));
    assert(timeline.milestone_count() == 2);

    // Try to remove non-existent milestone
    assert(!timeline.remove_milestone(3.0));

    auto milestones = timeline.get_milestones();
    assert(milestones[0].first == 2.0);
    assert(milestones[1].first == 7.0);

    std::cout << "  OK: Remove milestone works" << std::endl;
}

void test_clear_milestones() {
    std::cout << "Testing clear milestones..." << std::endl;

    Timeline timeline(0.0, 10.0);

    timeline.add_milestone(1.0, "A");
    timeline.add_milestone(3.0, "B");
    timeline.add_milestone(5.0, "C");

    assert(timeline.milestone_count() == 3);

    timeline.clear_milestones();
    assert(timeline.milestone_count() == 0);
    assert(timeline.get_milestones().empty());

    std::cout << "  OK: Clear milestones works" << std::endl;
}

void test_next_previous_milestone() {
    std::cout << "Testing next/previous milestone..." << std::endl;

    Timeline timeline(0.0, 10.0);

    timeline.add_milestone(2.0, "A");
    timeline.add_milestone(5.0, "B");
    timeline.add_milestone(8.0, "C");

    // Next milestone
    assert(timeline.get_next_milestone(1.0) == std::string("A"));
    assert(timeline.get_next_milestone(2.0) == std::string("B"));
    assert(timeline.get_next_milestone(5.0) == std::string("C"));
    assert(timeline.get_next_milestone(8.0) == nullptr);
    assert(timeline.get_next_milestone(10.0) == nullptr);

    // Previous milestone
    assert(timeline.get_previous_milestone(1.0) == nullptr);
    assert(timeline.get_previous_milestone(2.0) == nullptr);
    assert(timeline.get_previous_milestone(3.0) == std::string("A"));
    assert(timeline.get_previous_milestone(5.0) == std::string("A"));
    assert(timeline.get_previous_milestone(6.0) == std::string("B"));
    assert(timeline.get_previous_milestone(10.0) == std::string("C"));

    std::cout << "  OK: Next/previous milestone works" << std::endl;
}

void test_invalid_bounds() {
    std::cout << "Testing invalid bounds..." << std::endl;

    Timeline timeline;

    // Start >= end should throw
    bool threw = false;
    try {
        timeline.set_bounds(5.0, 5.0);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw);

    threw = false;
    try {
        timeline.set_bounds(10.0, 5.0);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw);

    // Negative bounds should throw
    threw = false;
    try {
        timeline.set_bounds(-1.0, 10.0);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw);

    std::cout << "  OK: Invalid bounds handling works" << std::endl;
}

void test_milestone_outside_bounds() {
    std::cout << "Testing milestone outside bounds..." << std::endl;

    Timeline timeline(0.0, 10.0);

    // Milestone outside bounds should throw
    bool threw = false;
    try {
        timeline.add_milestone(-1.0, "Too early");
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw);

    threw = false;
    try {
        timeline.add_milestone(11.0, "Too late");
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw);

    std::cout << "  OK: Milestone outside bounds handling works" << std::endl;
}

int main() {
    std::cout << "=== Timeline Tests ===" << std::endl;
    std::cout << std::endl;

    test_basic_timeline();
    test_contains_timestamp();
    test_set_bounds();
    test_add_milestone();
    test_milestone_ordering();
    test_update_milestone();
    test_get_milestones_in_range();
    test_remove_milestone();
    test_clear_milestones();
    test_next_previous_milestone();
    test_invalid_bounds();
    test_milestone_outside_bounds();

    std::cout << std::endl;
    std::cout << "=== All tests passed! ===" << std::endl;

    return 0;
}
