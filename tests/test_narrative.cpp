#include "cebu/simplicial_complex_narrative.h"
#include <iostream>
#include <cassert>

using namespace cebu;

void test_basic_narrative_complex() {
    std::cout << "Testing basic narrative complex..." << std::endl;

    SimplicialComplexNarrative<double> complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    SimplexID edge = complex.add_edge(v0, v1);

    // Set labels
    complex.set_label(v0, 0.5);
    complex.set_label(v1, 0.7);
    complex.set_label(edge, 0.6);

    // Timeline should have default bounds
    auto bounds = complex.timeline().get_bounds();
    assert(bounds.first == 0.0);
    assert(bounds.second == 1.0);

    assert(complex.current_time() == 0.0);

    std::cout << "  OK: Basic narrative complex works" << std::endl;
}

void test_add_and_apply_event() {
    std::cout << "Testing add and apply event..." << std::endl;

    SimplicialComplexNarrative<double> complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    SimplexID e01 = complex.add_edge(v0, v1);
    SimplexID e12 = complex.add_edge(v1, v2);

    // Set initial labels
    complex.set_label(e01, 0.5);
    complex.set_label(e12, 0.3);

    // Add event at t=0.5
    AbsurdityContext ctx{0.5, 0.3, 0.4, 0.6, 1.0};
    EventID event_id = complex.add_event(
        "Dramatic twist",
        0.5,
        {e01},
        ctx,
        false  // Don't apply now
    );

    assert(complex.events().event_count() == 1);
    assert(complex.get_label(e01) == 0.5);  // Label unchanged

    // Apply event
    const StoryEvent& event = complex.events().get_event(event_id);
    complex.apply_event(event);

    // For double labels, evolve_label doesn't modify (not absurdity type)
    assert(complex.get_label(e01) == 0.5);

    std::cout << "  OK: Add and apply event works" << std::endl;
}

void test_evolve_to_timestamp() {
    std::cout << "Testing evolve to timestamp..." << std::endl;

    SimplicialComplexNarrative<Absurdity> complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    SimplexID tri = complex.add_triangle(v0, v1, v2);

    // Set initial absurdity
    complex.set_label(tri, Absurdity(0.3, 0.4, 0.8));

    // Add multiple events
    AbsurdityContext ctx1{0.5, 0.3, 0.4, 0.6, 1.0};
    AbsurdityContext ctx2{0.8, 0.6, 0.7, 0.9, 1.0};

    complex.add_event("First shock", 0.3, {tri}, ctx1);
    complex.add_event("Second shock", 0.7, {tri}, ctx2);

    // Evolve to t=0.5 (should apply first event)
    complex.evolve_to(0.5);

    assert(complex.current_time() == 0.5);

    auto label = complex.get_label(tri);
    assert(label.has_value());
    // Absurdity should have changed after first event
    assert(label->midpoint() != 0.35);  // Was (0.3+0.4)/2 = 0.35

    std::cout << "  OK: Evolve to timestamp works" << std::endl;
}

void test_timeline_bounds_validation() {
    std::cout << "Testing timeline bounds validation..." << std::endl;

    SimplicialComplexNarrative<double> complex(0.0, 10.0);

    VertexID v0 = complex.add_vertex();

    // Event within bounds should work
    AbsurdityContext ctx{0.1, 0.2, 0.3, 0.4, 1.0};
    EventID id = complex.add_event("Valid", 5.0, {v0}, ctx);
    assert(complex.events().event_count() == 1);

    // Evolve to within bounds should work
    complex.evolve_to(5.0);
    assert(complex.current_time() == 5.0);

    // Evolve outside bounds should throw
    bool threw = false;
    try {
        complex.evolve_to(15.0);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw);

    std::cout << "  OK: Timeline bounds validation works" << std::endl;
}

void test_multiple_components() {
    std::cout << "Testing multiple components..." << std::endl;

    SimplicialComplexNarrative<double> complex;

    // First component
    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    SimplexID e01 = complex.add_edge(v0, v1);

    // Second component
    VertexID v2 = complex.add_vertex();
    VertexID v3 = complex.add_vertex();
    SimplexID e23 = complex.add_edge(v2, v3);

    // Set different labels
    complex.set_label(e01, 0.2);
    complex.set_label(e23, 0.8);

    // Add events affecting different components
    AbsurdityContext ctx{0.5, 0.3, 0.4, 0.6, 1.0};

    EventID event1 = complex.add_event("A", 0.3, {e01}, ctx);
    EventID event2 = complex.add_event("B", 0.7, {e23}, ctx);

    assert(complex.events().event_count() == 2);

    // Evolve and check
    complex.evolve_to(0.5);
    assert(complex.current_time() == 0.5);
    // Only event1 should have been applied

    std::cout << "  OK: Multiple components works" << std::endl;
}

void test_milestones() {
    std::cout << "Testing milestones..." << std::endl;

    SimplicialComplexNarrative<double> complex(0.0, 100.0);

    VertexID v0 = complex.add_vertex();

    complex.timeline().add_milestone(10.0, "Chapter 1");
    complex.timeline().add_milestone(50.0, "Midpoint");
    complex.timeline().add_milestone(90.0, "Chapter 2");

    assert(complex.timeline().milestone_count() == 3);

    // Get next milestone
    auto* next = complex.timeline().get_next_milestone(20.0);
    assert(next != nullptr);
    assert(*next == "Midpoint");

    // Get previous milestone
    auto* prev = complex.timeline().get_previous_milestone(60.0);
    assert(prev != nullptr);
    assert(*prev == "Midpoint");

    std::cout << "  OK: Milestones work" << std::endl;
}

void test_reset() {
    std::cout << "Testing reset..." << std::endl;

    SimplicialComplexNarrative<double> complex(0.0, 10.0);

    VertexID v0 = complex.add_vertex();

    // Evolve to some time
    complex.evolve_to(5.0);
    assert(complex.current_time() == 5.0);

    // Reset
    complex.reset();
    assert(complex.current_time() == 0.0);

    std::cout << "  OK: Reset works" << std::endl;
}

void test_custom_timeline() {
    std::cout << "Testing custom timeline..." << std::endl;

    SimplicialComplexNarrative<double> complex(10.0, 20.0);

    VertexID v0 = complex.add_vertex();

    auto bounds = complex.timeline().get_bounds();
    assert(bounds.first == 10.0);
    assert(bounds.second == 20.0);
    assert(complex.timeline().duration() == 10.0);

    // Event should be within custom bounds
    AbsurdityContext ctx{0.1, 0.2, 0.3, 0.4, 1.0};
    EventID id = complex.add_event("Event", 15.0, {v0}, ctx);
    assert(complex.events().event_count() == 1);

    // Evolve should work
    complex.evolve_to(15.0);
    assert(complex.current_time() == 15.0);

    std::cout << "  OK: Custom timeline works" << std::endl;
}

int main() {
    std::cout << "=== Narrative Complex Tests ===" << std::endl;
    std::cout << std::endl;

    test_basic_narrative_complex();
    test_add_and_apply_event();
    test_evolve_to_timestamp();
    test_timeline_bounds_validation();
    test_multiple_components();
    test_milestones();
    test_reset();
    test_custom_timeline();

    std::cout << std::endl;
    std::cout << "=== All tests passed! ===" << std::endl;

    return 0;
}
