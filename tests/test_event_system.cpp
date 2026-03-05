#include "cebu/event_system.h"
#include "cebu/simplicial_complex_labeled_events.h"
#include <cassert>
#include <iostream>
#include <string>

using namespace cebu;

// Test counters
static int add_simplex_count = 0;
static int remove_simplex_count = 0;
static int label_change_count = 0;
static int topology_change_count = 0;
static int refine_begin_count = 0;
static int refine_end_count = 0;
static int coarsen_begin_count = 0;
static int coarsen_end_count = 0;
static int glue_count = 0;
static int separate_count = 0;
static int morph_begin_count = 0;
static int morph_end_count = 0;

void reset_counters() {
    add_simplex_count = 0;
    remove_simplex_count = 0;
    label_change_count = 0;
    topology_change_count = 0;
    refine_begin_count = 0;
    refine_end_count = 0;
    coarsen_begin_count = 0;
    coarsen_end_count = 0;
    glue_count = 0;
    separate_count = 0;
    morph_begin_count = 0;
    morph_end_count = 0;
}

void test_event_system_basic() {
    std::cout << "Test: Event System Basic Operations\n";

    EventSystem<double> events;

    int callback_count = 0;
    SimplexID captured_id = 0;

    auto callback_id = events.register_callback(EventType::ON_ADD_SIMPLEX,
        [&callback_count, &captured_id](const EventData& event, const double* label) {
            callback_count++;
            captured_id = event.simplex_id;
        });

    // Trigger event
    EventData event(EventType::ON_ADD_SIMPLEX, 123, 1, "Test edge");
    events.trigger_event(event);

    assert(callback_count == 1);
    assert(captured_id == 123);

    // Unregister
    bool unregistered = events.unregister_callback(EventType::ON_ADD_SIMPLEX, callback_id);
    assert(unregistered);

    // Trigger again - should not call callback
    events.trigger_event(event);
    assert(callback_count == 1);

    std::cout << "  ✓ Basic event system operations work\n\n";
}

void test_event_system_multiple_callbacks() {
    std::cout << "Test: Multiple Callbacks\n";

    EventSystem<double> events;

    int count1 = 0, count2 = 0, count3 = 0;

    events.register_callback(EventType::ON_REMOVE_SIMPLEX,
        [&count1](const EventData&, const double*) { count1++; });

    events.register_callback(EventType::ON_REMOVE_SIMPLEX,
        [&count2](const EventData&, const double*) { count2++; });

    events.register_callback(EventType::ON_REMOVE_SIMPLEX,
        [&count3](const EventData&, const double*) { count3++; });

    EventData event(EventType::ON_REMOVE_SIMPLEX, 456, 2, "Test triangle");
    events.trigger_event(event);

    assert(count1 == 1);
    assert(count2 == 1);
    assert(count3 == 1);

    // Check callback count
    assert(events.callback_count(EventType::ON_REMOVE_SIMPLEX) == 3);
    assert(events.has_callbacks(EventType::ON_REMOVE_SIMPLEX));

    std::cout << "  ✓ Multiple callbacks work correctly\n\n";
}

void test_event_system_batching() {
    std::cout << "Test: Event Batching\n";

    EventSystem<double> events;

    int count = 0;

    events.register_callback(EventType::ON_ADD_SIMPLEX,
        [&count](const EventData&, const double*) { count++; });

    // Enable batching
    events.set_batching(true);

    // Trigger multiple events
    for (int i = 0; i < 5; ++i) {
        EventData event(EventType::ON_ADD_SIMPLEX, i, 0, "Vertex");
        events.trigger_event(event);
    }

    // Events should be queued, not executed
    assert(count == 0);

    // Flush events
    events.flush_events();

    // Now all events should be executed
    assert(count == 5);

    std::cout << "  ✓ Event batching works correctly\n\n";
}

void test_event_system_enable_disable() {
    std::cout << "Test: Enable/Disable Events\n";

    EventSystem<double> events;

    int count = 0;

    events.register_callback(EventType::ON_LABEL_CHANGE,
        [&count](const EventData&, const double*) { count++; });

    // Disable events
    events.set_enabled(false);

    EventData event(EventType::ON_LABEL_CHANGE, 789, 1, "Label change");
    events.trigger_event(event);

    assert(count == 0);

    // Enable events
    events.set_enabled(true);
    events.trigger_event(event);

    assert(count == 1);

    std::cout << "  ✓ Enable/disable works correctly\n\n";
}

void test_labeled_complex_events() {
    std::cout << "Test: Labeled Complex Events\n";

    reset_counters();

    SimplicialComplexLabeledEvents<double> complex;

    // Register callbacks
    complex.register_callback(EventType::ON_ADD_SIMPLEX,
        [](const EventData& event, const double* label) {
            add_simplex_count++;
        });

    complex.register_callback(EventType::ON_REMOVE_SIMPLEX,
        [](const EventData& event, const double* label) {
            remove_simplex_count++;
        });

    complex.register_callback(EventType::ON_LABEL_CHANGE,
        [](const EventData& event, const double* label) {
            label_change_count++;
        });

    // Add simplices
    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    SimplexID edge = complex.add_edge(v0, v1);
    SimplexID tri = complex.add_triangle(v0, v1, v2);

    assert(add_simplex_count == 5); // 3 vertices + 1 edge + 1 triangle

    // Set labels
    complex.set_label(edge, 0.5);
    complex.set_label(tri, 0.8);
    complex.set_label(edge, 0.6);  // Update label

    assert(label_change_count == 3);

    // Remove simplex
    complex.remove_simplex(edge);
    assert(remove_simplex_count == 1);

    // Remove label
    complex.remove_label(tri);
    assert(label_change_count == 4); // +1 for label removal

    std::cout << "  ✓ Labeled complex events work correctly\n\n";
}

void test_generic_callbacks() {
    std::cout << "Test: Generic Callbacks\n";

    EventSystem<double> events;

    int generic_count = 0;
    int typed_count = 0;

    // Register generic callback
    events.register_callback(EventType::ON_ADD_SIMPLEX,
        [&generic_count](const EventData& event) {
            generic_count++;
        });

    // Register typed callback
    events.register_callback(EventType::ON_ADD_SIMPLEX,
        [&typed_count](const EventData& event, const double* label) {
            typed_count++;
        });

    EventData event(EventType::ON_ADD_SIMPLEX, 100, 0, "Vertex");
    events.trigger_event(event);

    assert(generic_count == 1);
    assert(typed_count == 1);

    std::cout << "  ✓ Generic callbacks work correctly\n\n";
}

void test_clear_callbacks() {
    std::cout << "Test: Clear Callbacks\n";

    EventSystem<double> events;

    events.register_callback(EventType::ON_ADD_SIMPLEX,
        [](const EventData&, const double*) {});

    events.register_callback(EventType::ON_ADD_SIMPLEX,
        [](const EventData&, const double*) {});

    events.register_callback(EventType::ON_REMOVE_SIMPLEX,
        [](const EventData&, const double*) {});

    assert(events.callback_count(EventType::ON_ADD_SIMPLEX) == 2);
    assert(events.callback_count(EventType::ON_REMOVE_SIMPLEX) == 1);

    // Clear specific type
    events.clear_callbacks(EventType::ON_ADD_SIMPLEX);
    assert(events.callback_count(EventType::ON_ADD_SIMPLEX) == 0);
    assert(events.callback_count(EventType::ON_REMOVE_SIMPLEX) == 1);

    // Clear all
    events.clear_all_callbacks();
    assert(events.callback_count(EventType::ON_ADD_SIMPLEX) == 0);
    assert(events.callback_count(EventType::ON_REMOVE_SIMPLEX) == 0);

    std::cout << "  ✓ Clear callbacks works correctly\n\n";
}

void test_topology_change_events() {
    std::cout << "Test: Topology Change Events\n";

    reset_counters();

    SimplicialComplexLabeledEvents<double> complex;

    complex.register_callback(EventType::ON_TOPOLOGY_CHANGE,
        [](const EventData& event, const double* label) {
            topology_change_count++;
        });

    // Notify batch operations
    complex.notify_topology_change_begin("Test operation", {1, 2, 3});
    complex.notify_topology_change_end("Test operation", {1, 2, 3});

    assert(topology_change_count == 2);

    std::cout << "  ✓ Topology change events work correctly\n\n";
}

void test_refinement_events() {
    std::cout << "Test: Refinement Events\n";

    reset_counters();

    SimplicialComplexLabeledEvents<double> complex;

    complex.register_callback(EventType::ON_REFINE_BEGIN,
        [](const EventData& event, const double* label) {
            refine_begin_count++;
        });

    complex.register_callback(EventType::ON_REFINE_END,
        [](const EventData& event, const double* label) {
            refine_end_count++;
        });

    // Notify refinement
    complex.notify_refine_begin(100, 1);
    complex.notify_refine_end(100, 1, 4, 3);

    assert(refine_begin_count == 1);
    assert(refine_end_count == 1);

    std::cout << "  ✓ Refinement events work correctly\n\n";
}

void test_coarsening_events() {
    std::cout << "Test: Coarsening Events\n";

    reset_counters();

    SimplicialComplexLabeledEvents<double> complex;

    complex.register_callback(EventType::ON_COARSEN_BEGIN,
        [](const EventData& event, const double* label) {
            coarsen_begin_count++;
        });

    complex.register_callback(EventType::ON_COARSEN_END,
        [](const EventData& event, const double* label) {
            coarsen_end_count++;
        });

    // Notify coarsening
    complex.notify_coarsen_begin(200, 2);
    complex.notify_coarsen_end(200, 2, 3, 1);

    assert(coarsen_begin_count == 1);
    assert(coarsen_end_count == 1);

    std::cout << "  ✓ Coarsening events work correctly\n\n";
}

void test_glue_separate_events() {
    std::cout << "Test: Glue/Separate Events\n";

    reset_counters();

    SimplicialComplexLabeledEvents<double> complex;

    complex.register_callback(EventType::ON_GLUE,
        [](const EventData& event, const double* label) {
            glue_count++;
        });

    complex.register_callback(EventType::ON_SEPARATE,
        [](const EventData& event, const double* label) {
            separate_count++;
        });

    // Notify glue
    complex.notify_glue(10, 20, "Glued vertices");
    assert(glue_count == 1);

    // Notify separate
    complex.notify_separate(10, 20, "Separated vertices");
    assert(separate_count == 1);

    std::cout << "  ✓ Glue/separate events work correctly\n\n";
}

void test_morph_events() {
    std::cout << "Test: Morph Events\n";

    reset_counters();

    SimplicialComplexLabeledEvents<double> complex;

    complex.register_callback(EventType::ON_MORPH_BEGIN,
        [](const EventData& event, const double* label) {
            morph_begin_count++;
        });

    complex.register_callback(EventType::ON_MORPH_END,
        [](const EventData& event, const double* label) {
            morph_end_count++;
        });

    // Notify morph
    complex.notify_morph_begin("Test morph", {1, 2, 3});
    complex.notify_morph_end("Test morph", {1, 2, 3});

    assert(morph_begin_count == 1);
    assert(morph_end_count == 1);

    std::cout << "  ✓ Morph events work correctly\n\n";
}

void test_event_data_structures() {
    std::cout << "Test: Event Data Structures\n";

    // Basic EventData
    EventData event1(EventType::ON_ADD_SIMPLEX, 100, 1, "Edge added");
    assert(event1.type == EventType::ON_ADD_SIMPLEX);
    assert(event1.simplex_id == 100);
    assert(event1.dimension == 1);
    assert(event1.description == "Edge added");

    // LabelChangeEvent
    LabelChangeEvent label_event(200, 2, true, true);
    assert(label_event.type == EventType::ON_LABEL_CHANGE);
    assert(label_event.label_was_added);
    assert(label_event.label_exists_after);

    // RefinementEvent
    RefinementEvent refine_event(300, 1, 4, 2, false);
    assert(refine_event.type == EventType::ON_REFINE_END);
    assert(refine_event.refinement_level == 1);
    assert(refine_event.new_simplices_count == 4);
    assert(refine_event.new_vertices_count == 2);

    // CoarseningEvent
    CoarseningEvent coarse_event(400, 2, 3, 1, false);
    assert(coarse_event.type == EventType::ON_COARSEN_END);
    assert(coarse_event.refinement_level == 2);
    assert(coarse_event.removed_simplices_count == 3);
    assert(coarse_event.removed_vertices_count == 1);

    // GlueEvent
    GlueEvent glue_event(10, 20, "Glued");
    assert(glue_event.type == EventType::ON_GLUE);
    assert(glue_event.simplex_id == 10);
    assert(glue_event.related_id.has_value());
    assert(*glue_event.related_id == 20);

    // SeparateEvent
    SeparateEvent separate_event(10, 20, "Separated");
    assert(separate_event.type == EventType::ON_SEPARATE);
    assert(separate_event.simplex_id == 10);
    assert(separate_event.related_id.has_value());
    assert(*separate_event.related_id == 20);

    // TopologyChangeEvent
    TopologyChangeEvent top_event("Batch op", true, {1, 2, 3});
    assert(top_event.type == EventType::ON_TOPOLOGY_CHANGE);
    assert(top_event.operation_name == "Batch op");
    assert(top_event.is_begin);
    assert(top_event.affected_simplices.size() == 3);

    // Event type to string
    assert(std::string(event_type_to_string(EventType::ON_ADD_SIMPLEX)) == "ON_ADD_SIMPLEX");
    assert(std::string(event_type_to_string(EventType::ON_REMOVE_SIMPLEX)) == "ON_REMOVE_SIMPLEX");
    assert(std::string(event_type_to_string(EventType::ON_LABEL_CHANGE)) == "ON_LABEL_CHANGE");

    std::cout << "  ✓ Event data structures work correctly\n\n";
}

void test_event_system_string_labels() {
    std::cout << "Test: Event System with String Labels\n";

    EventSystem<std::string> events;

    std::string captured_label;

    events.register_callback(EventType::ON_LABEL_CHANGE,
        [&captured_label](const EventData& event, const std::string* label) {
            if (label) {
                captured_label = *label;
            }
        });

    std::string test_label = "test_label";
    EventData event(EventType::ON_LABEL_CHANGE, 100, 1, "Label changed");
    events.trigger_event(event, &test_label);

    assert(captured_label == "test_label");

    std::cout << "  ✓ Event system works with string labels\n\n";
}

int main() {
    std::cout << "=== Cebu Event System Tests ===\n\n";

    try {
        test_event_system_basic();
        test_event_system_multiple_callbacks();
        test_event_system_batching();
        test_event_system_enable_disable();
        test_generic_callbacks();
        test_clear_callbacks();
        test_labeled_complex_events();
        test_topology_change_events();
        test_refinement_events();
        test_coarsening_events();
        test_glue_separate_events();
        test_morph_events();
        test_event_data_structures();
        test_event_system_string_labels();

        std::cout << "=== All Tests Passed! ===\n";
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << "\n";
        return 1;
    }
}
