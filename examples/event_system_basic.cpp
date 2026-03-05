// Basic Event System Example for Cebu Library
// This example demonstrates event subscription and handling in simplicial complexes

#include "cebu/simplicial_complex_labeled_events.h"
#include "cebu/event_system.h"
#include <iostream>
#include <iomanip>

using namespace cebu;

// Custom event logger class
class EventLogger {
private:
    int event_count_ = 0;

public:
    void log_event(const EventData& event, const double* label) {
        event_count_++;
        std::cout << "[Event #" << std::setw(3) << event_count_ << "] "
                  << event_type_to_string(event.type) << " - "
                  << event.description;

        if (event.simplex_id != 0) {
            std::cout << " (SimplexID: " << event.simplex_id << ")";
        }

        if (label) {
            std::cout << " (Label: " << std::fixed << std::setprecision(2) << *label << ")";
        }

        std::cout << "\n";
    }

    int event_count() const { return event_count_; }
};

void example_1_basic_events() {
    std::cout << "\n=== Example 1: Basic Event Handling ===\n";
    std::cout << std::string(50, '=') << "\n\n";

    SimplicialComplexLabeledEvents<double> complex;
    EventLogger logger;

    // Register callback for all simplex additions
    complex.register_callback(EventType::ON_ADD_SIMPLEX,
        [&logger](const EventData& event, const double* label) {
            logger.log_event(event, label);
        });

    std::cout << "Adding simplices...\n";

    // Create simplices
    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    SimplexID edge = complex.add_edge(v0, v1);
    SimplexID tri = complex.add_triangle(v0, v1, v2);

    std::cout << "\nTotal events logged: " << logger.event_count() << "\n";
}

void example_2_label_events() {
    std::cout << "\n=== Example 2: Label Change Events ===\n";
    std::cout << std::string(50, '=') << "\n\n";

    SimplicialComplexLabeledEvents<double> complex;

    // Register label change callback
    complex.register_callback(EventType::ON_LABEL_CHANGE,
        [](const EventData& event, const double* label) {
            const LabelChangeEvent* label_event = static_cast<const LabelChangeEvent*>(&event);
            std::cout << "Label " << (label_event->label_was_added ? "added" : "changed/removed")
                      << " for simplex " << event.simplex_id;

            if (label) {
                std::cout << ", new value: " << std::fixed << std::setprecision(2) << *label;
            }

            if (!label_event->label_exists_after) {
                std::cout << " (removed)";
            }

            std::cout << "\n";
        });

    std::cout << "Creating triangle...\n";
    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    SimplexID tri = complex.add_triangle(v0, v1, v2);

    std::cout << "\nAdding labels...\n";
    complex.set_label(tri, 0.8);
    complex.set_label(tri, 0.9);  // Update
    complex.set_label(v0, 0.5);

    std::cout << "\nRemoving label...\n";
    complex.remove_label(v0);
}

void example_3_removal_events() {
    std::cout << "\n=== Example 3: Removal Events ===\n";
    std::cout << std::string(50, '=') << "\n\n";

    SimplicialComplexLabeledEvents<double> complex;

    // Register callbacks
    complex.register_callback(EventType::ON_ADD_SIMPLEX,
        [](const EventData& event, const double*) {
            std::cout << "Added simplex " << event.simplex_id
                      << " (dim " << event.dimension << ")\n";
        });

    complex.register_callback(EventType::ON_REMOVE_SIMPLEX,
        [](const EventData& event, const double* label) {
            std::cout << "Removed simplex " << event.simplex_id;
            if (label) {
                std::cout << " (had label: " << std::fixed << std::setprecision(2) << *label << ")";
            }
            std::cout << "\n";
        });

    std::cout << "Creating simplices...\n";
    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    SimplexID edge = complex.add_edge(v0, v1);

    complex.set_label(edge, 0.7);

    std::cout << "\nRemoving edge...\n";
    complex.remove_simplex(edge);
}

void example_4_batch_operations() {
    std::cout << "\n=== Example 4: Batch Operations ===\n";
    std::cout << std::string(50, '=') << "\n\n";

    SimplicialComplexLabeledEvents<double> complex;

    // Register batch operation callback
    complex.register_callback(EventType::ON_TOPOLOGY_CHANGE,
        [](const EventData& event, const double*) {
            const TopologyChangeEvent* topo_event = static_cast<const TopologyChangeEvent*>(&event);
            std::cout << "Batch operation: " << topo_event->operation_name
                      << " - " << (topo_event->is_begin ? "BEGIN" : "END");

            if (!topo_event->affected_simplices.empty()) {
                std::cout << " (affects " << topo_event->affected_simplices.size() << " simplices)";
            }

            std::cout << "\n";
        });

    std::cout << "Performing batch operations...\n";

    // Simulate batch operation
    complex.notify_topology_change_begin("Load mesh", {1, 2, 3, 4, 5});

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    complex.add_edge(v0, v1);
    complex.add_edge(v1, v2);
    complex.add_triangle(v0, v1, v2);

    complex.notify_topology_change_end("Load mesh", {v0, v1, v2});
}

void example_5_multiple_callbacks() {
    std::cout << "\n=== Example 5: Multiple Callbacks ===\n";
    std::cout << std::string(50, '=') << "\n\n";

    SimplicialComplexLabeledEvents<double> complex;

    // Register multiple callbacks for same event type
    int callback1_count = 0;
    int callback2_count = 0;
    int callback3_count = 0;

    auto cb1_id = complex.register_callback(EventType::ON_ADD_SIMPLEX,
        [&callback1_count](const EventData&, const double*) {
            callback1_count++;
        });

    auto cb2_id = complex.register_callback(EventType::ON_ADD_SIMPLEX,
        [&callback2_count](const EventData&, const double*) {
            callback2_count++;
        });

    auto cb3_id = complex.register_callback(EventType::ON_ADD_SIMPLEX,
        [&callback3_count](const EventData&, const double*) {
            callback3_count++;
        });

    std::cout << "Adding 3 vertices (each triggers all 3 callbacks)...\n";
    complex.add_vertex();
    complex.add_vertex();
    complex.add_vertex();

    std::cout << "Callback 1 called: " << callback1_count << " times\n";
    std::cout << "Callback 2 called: " << callback2_count << " times\n";
    std::cout << "Callback 3 called: " << callback3_count << " times\n";

    std::cout << "\nUnregistering callback 2...\n";
    complex.unregister_callback(EventType::ON_ADD_SIMPLEX, cb2_id);

    std::cout << "Adding 1 more vertex...\n";
    complex.add_vertex();

    std::cout << "Callback 1 called: " << callback1_count << " times\n";
    std::cout << "Callback 2 called: " << callback2_count << " times\n";
    std::cout << "Callback 3 called: " << callback3_count << " times\n";
}

void example_6_event_batching() {
    std::cout << "\n=== Example 6: Event Batching ===\n";
    std::cout << std::string(50, '=') << "\n\n";

    SimplicialComplexLabeledEvents<double> complex;

    int event_count = 0;

    complex.register_callback(EventType::ON_ADD_SIMPLEX,
        [&event_count](const EventData&, const double*) {
            event_count++;
        });

    std::cout << "Adding simplices with batching enabled...\n";

    // Enable batching
    complex.set_events_enabled(false);  // Temporarily disable
    complex.get_event_system().set_batching(true);

    // Add simplices (events are queued)
    complex.add_vertex();
    complex.add_vertex();
    complex.add_vertex();
    complex.add_edge(0, 1);
    complex.add_edge(1, 2);

    std::cout << "Simplices added, events queued\n";
    std::cout << "Events triggered so far: " << event_count << "\n";

    // Flush events
    complex.set_events_enabled(true);
    complex.get_event_system().flush_events();

    std::cout << "After flushing: " << event_count << " events\n";
}

void example_7_refinement_events() {
    std::cout << "\n=== Example 7: Refinement Events ===\n";
    std::cout << std::string(50, '=') << "\n\n";

    SimplicialComplexLabeledEvents<double> complex;

    complex.register_callback(EventType::ON_REFINE_BEGIN,
        [](const EventData& event, const double* label) {
            const RefinementEvent* refine_event = static_cast<const RefinementEvent*>(&event);
            std::cout << "Refinement BEGIN for simplex " << event.simplex_id
                      << " at level " << refine_event->refinement_level << "\n";
        });

    complex.register_callback(EventType::ON_REFINE_END,
        [](const EventData& event, const double* label) {
            const RefinementEvent* refine_event = static_cast<const RefinementEvent*>(&event);
            std::cout << "Refinement END for simplex " << event.simplex_id
                      << " - Created " << refine_event->new_simplices_count
                      << " simplices, " << refine_event->new_vertices_count
                      << " vertices\n";
        });

    std::cout << "Simulating refinement operations...\n";

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    SimplexID tri = complex.add_triangle(v0, v1, v2);

    complex.set_label(tri, 0.9);

    // Simulate refinement
    complex.notify_refine_begin(tri, 1);
    complex.notify_refine_end(tri, 1, 4, 3);
}

void example_8_generic_callbacks() {
    std::cout << "\n=== Example 8: Generic Callbacks ===\n";
    std::cout << std::string(50, '=') << "\n\n";

    EventSystem<double> events;

    // Register generic callback (no label access)
    int generic_count = 0;
    events.register_callback(EventType::ON_ADD_SIMPLEX,
        [&generic_count](const EventData& event) {
            generic_count++;
            std::cout << "[Generic] Event " << event_type_to_string(event.type)
                      << " for simplex " << event.simplex_id << "\n";
        });

    // Register typed callback (with label access)
    int typed_count = 0;
    events.register_callback(EventType::ON_ADD_SIMPLEX,
        [&typed_count](const EventData& event, const double* label) {
            typed_count++;
            if (label) {
                std::cout << "[Typed] Event " << event_type_to_string(event.type)
                          << " for simplex " << event.simplex_id
                          << " with label " << std::fixed << std::setprecision(2) << *label << "\n";
            }
        });

    std::cout << "Triggering events...\n";

    EventData event1(EventType::ON_ADD_SIMPLEX, 100, 0, "Vertex");
    double label1 = 0.5;
    events.trigger_event(event1, &label1);

    EventData event2(EventType::ON_ADD_SIMPLEX, 101, 1, "Edge");
    events.trigger_event(event2, nullptr);  // No label

    std::cout << "\nGeneric callback called: " << generic_count << " times\n";
    std::cout << "Typed callback called: " << typed_count << " times\n";
}

void example_9_conditional_callbacks() {
    std::cout << "\n=== Example 9: Conditional Callbacks ===\n";
    std::cout << std::string(50, '=') << "\n\n";

    SimplicialComplexLabeledEvents<double> complex;

    // Callback that only logs high-label events
    complex.register_callback(EventType::ON_LABEL_CHANGE,
        [](const EventData& event, const double* label) {
            if (label && *label > 0.7) {
                std::cout << "High label event! Simplex " << event.simplex_id
                          << " has label " << std::fixed << std::setprecision(2) << *label << "\n";
            }
        });

    // Callback that logs dimension-specific events
    complex.register_callback(EventType::ON_ADD_SIMPLEX,
        [](const EventData& event, const double*) {
            std::string dim_name = event.dimension == 0 ? "vertex" :
                                   event.dimension == 1 ? "edge" :
                                   event.dimension == 2 ? "triangle" : "simplex";
            std::cout << "Added " << dim_name << " " << event.simplex_id << "\n";
        });

    std::cout << "Creating simplices...\n";
    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    SimplexID edge = complex.add_edge(v0, v1);

    std::cout << "\nSetting labels...\n";
    complex.set_label(v0, 0.5);   // Low label (won't trigger high-label callback)
    complex.set_label(v1, 0.8);   // High label (will trigger)
    complex.set_label(edge, 0.9); // High label (will trigger)
}

void example_10_event_filtering() {
    std::cout << "\n=== Example 10: Event Filtering ===\n";
    std::cout << std::string(50, '=') << "\n\n";

    SimplicialComplexLabeledEvents<double> complex;

    // Only log triangle events
    complex.register_callback(EventType::ON_ADD_SIMPLEX,
        [](const EventData& event, const double*) {
            if (event.dimension == 2) {
                std::cout << "Triangle " << event.simplex_id << " added\n";
            }
        });

    // Only log vertex removals
    complex.register_callback(EventType::ON_REMOVE_SIMPLEX,
        [](const EventData& event, const double*) {
            if (event.dimension == 0) {
                std::cout << "Vertex " << event.simplex_id << " removed\n";
            }
        });

    std::cout << "Creating simplices...\n";
    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    SimplexID tri = complex.add_triangle(v0, v1, v2);

    std::cout << "\nRemoving simplices...\n";
    complex.remove_simplex(v0);  // Vertex removal logged
    complex.remove_simplex(tri); // Triangle removal NOT logged (dimension filter)
}

int main() {
    std::cout << "=== Cebu Event System Examples ===\n\n";

    example_1_basic_events();
    example_2_label_events();
    example_3_removal_events();
    example_4_batch_operations();
    example_5_multiple_callbacks();
    example_6_event_batching();
    example_7_refinement_events();
    example_8_generic_callbacks();
    example_9_conditional_callbacks();
    example_10_event_filtering();

    std::cout << "\n=== Summary ===\n";
    std::cout << "These examples demonstrate:\n";
    std::cout << "  1. Basic event handling\n";
    std::cout << "  2. Label change events\n";
    std::cout << "  3. Removal events\n";
    std::cout << "  4. Batch operation notifications\n";
    std::cout << "  5. Multiple callbacks per event\n";
    std::cout << "  6. Event batching\n";
    std::cout << "  7. Refinement events\n";
    std::cout << "  8. Generic vs typed callbacks\n";
    std::cout << "  9. Conditional callbacks\n";
    std::cout << "  10. Event filtering\n";
    std::cout << "\nThe event system enables loose coupling between the\n";
    std::cout << "simplicial complex and external systems like renderers.\n\n";

    return 0;
}
