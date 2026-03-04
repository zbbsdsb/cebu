#include "cebu/story_event.h"
#include "cebu/absurdity.h"
#include <iostream>
#include <cassert>
#include <algorithm>

using namespace cebu;

void test_add_and_retrieve_event() {
    std::cout << "Testing add and retrieve event..." << std::endl;

    StoryEventSystem system;

    AbsurdityContext context{0.5, 0.3, 0.2, 0.4, 1.0};
    EventID id = system.add_event("First event", 0.5, {1, 2, 3}, context);

    assert(system.event_count() == 1);

    const StoryEvent& event = system.get_event(id);
    assert(event.id == id);
    assert(event.description == "First event");
    assert(event.timestamp == 0.5);
    assert(event.affected_simplices.size() == 3);
    assert(event.impact.surprisal == 0.5);

    std::cout << "  OK: Add and retrieve event works" << std::endl;
}

void test_add_multiple_events() {
    std::cout << "Testing add multiple events..." << std::endl;

    StoryEventSystem system;

    AbsurdityContext ctx1{0.2, 0.1, 0.3, 0.4, 1.0};
    AbsurdityContext ctx2{0.6, 0.5, 0.7, 0.8, 1.0};

    EventID id1 = system.add_event("Event 1", 0.1, {1}, ctx1);
    EventID id2 = system.add_event("Event 2", 0.5, {2, 3}, ctx2);

    assert(system.event_count() == 2);
    assert(id1 != id2);

    std::cout << "  OK: Multiple events work" << std::endl;
}

void test_get_events_in_range() {
    std::cout << "Testing get events in range..." << std::endl;

    StoryEventSystem system;

    AbsurdityContext context{0.1, 0.2, 0.3, 0.4, 1.0};

    system.add_event("Early", 0.1, {1}, context);
    system.add_event("Middle 1", 0.3, {2}, context);
    system.add_event("Middle 2", 0.5, {3}, context);
    system.add_event("Late", 0.8, {4}, context);

    // Get middle events
    std::vector<StoryEvent> result = system.get_events_in_range(0.2, 0.6);
    assert(result.size() == 2);
    assert(result[0].description == "Middle 1");
    assert(result[1].description == "Middle 2");

    // Verify sorted order
    assert(result[0].timestamp < result[1].timestamp);

    std::cout << "  OK: Get events in range works" << std::endl;
}

void test_remove_event() {
    std::cout << "Testing remove event..." << std::endl;

    StoryEventSystem system;

    AbsurdityContext context{0.1, 0.2, 0.3, 0.4, 1.0};
    EventID id = system.add_event("To remove", 0.5, {1, 2, 3}, context);

    assert(system.event_count() == 1);

    system.remove_event(id);
    assert(system.event_count() == 0);

    // Try to get removed event should throw
    bool threw = false;
    try {
        system.get_event(id);
    } catch (const std::out_of_range&) {
        threw = true;
    }
    assert(threw);

    std::cout << "  OK: Remove event works" << std::endl;
}

void test_clear_all_events() {
    std::cout << "Testing clear all events..." << std::endl;

    StoryEventSystem system;

    AbsurdityContext context{0.1, 0.2, 0.3, 0.4, 1.0};
    system.add_event("Event 1", 0.1, {1}, context);
    system.add_event("Event 2", 0.2, {2}, context);
    system.add_event("Event 3", 0.3, {3}, context);

    assert(system.event_count() == 3);

    system.clear();
    assert(system.event_count() == 0);
    assert(system.get_all_events().size() == 0);

    std::cout << "  OK: Clear all events works" << std::endl;
}

void test_invalid_timestamp() {
    std::cout << "Testing invalid timestamp..." << std::endl;

    StoryEventSystem system;

    AbsurdityContext context{0.1, 0.2, 0.3, 0.4, 1.0};

    // Negative timestamp should throw
    bool threw = false;
    try {
        system.add_event("Invalid", -0.5, {1}, context);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw);

    std::cout << "  OK: Invalid timestamp handling works" << std::endl;
}

void test_invalid_range() {
    std::cout << "Testing invalid range..." << std::endl;

    StoryEventSystem system;

    AbsurdityContext context{0.1, 0.2, 0.3, 0.4, 1.0};
    system.add_event("Valid", 0.5, {1}, context);

    // Start > end should throw
    bool threw = false;
    try {
        system.get_events_in_range(1.0, 0.5);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw);

    std::cout << "  OK: Invalid range handling works" << std::endl;
}

void test_get_non_existent_event() {
    std::cout << "Testing get non-existent event..." << std::endl;

    StoryEventSystem system;

    // Get non-existent event should throw
    bool threw = false;
    try {
        system.get_event(999);
    } catch (const std::out_of_range&) {
        threw = true;
    }
    assert(threw);

    std::cout << "  OK: Non-existent event handling works" << std::endl;
}

void test_remove_non_existent_event() {
    std::cout << "Testing remove non-existent event..." << std::endl;

    StoryEventSystem system;

    // Remove non-existent event should throw
    bool threw = false;
    try {
        system.remove_event(999);
    } catch (const std::out_of_range&) {
        threw = true;
    }
    assert(threw);

    std::cout << "  OK: Non-existent event removal works" << std::endl;
}

int main() {
    std::cout << "=== Story Event System Tests ===" << std::endl;
    std::cout << std::endl;

    test_add_and_retrieve_event();
    test_add_multiple_events();
    test_get_events_in_range();
    test_remove_event();
    test_clear_all_events();
    test_invalid_timestamp();
    test_invalid_range();
    test_get_non_existent_event();
    test_remove_non_existent_event();

    std::cout << std::endl;
    std::cout << "=== All tests passed! ===" << std::endl;

    return 0;
}
