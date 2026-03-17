#pragma once

#include "cebu/simplicial_complex_labeled.h"
#include "cebu/event_system.h"

namespace cebu {

/**
 * @brief Simplicial complex with labels and event system
 *
 * Extends SimplicialComplexLabeled with event emission capabilities.
 * All topology and label changes trigger events that can be subscribed to.
 *
 * @tparam LabelType Type of the label
 */
template<typename LabelType>
class SimplicialComplexLabeledEvents : public SimplicialComplexLabeled<LabelType>, public EventEmitter<LabelType> {
public:
    /**
     * @brief Constructor
     */
    explicit SimplicialComplexLabeledEvents()
        : SimplicialComplexLabeled<LabelType>() {}

    explicit SimplicialComplexLabeledEvents(std::unique_ptr<LabelSystem<LabelType>> label_system)
        : SimplicialComplexLabeled<LabelType>(std::move(label_system)) {}

    // Override add operations to trigger events
    /**
     * @brief Add a vertex and trigger event
     */
    VertexID add_vertex() {
        VertexID vid = SimplicialComplexLabeled<LabelType>::add_vertex();
        if (this->events_enabled()) {
            EventData event(EventType::ON_ADD_SIMPLEX, vid, 0, "Vertex added");
            this->event_system_.trigger_event(event, nullptr);
        }
        return vid;
    }

    /**
     * @brief Add an edge and trigger event
     */
    SimplexID add_edge(VertexID v1, VertexID v2) {
        SimplexID eid = SimplicialComplexLabeled<LabelType>::add_edge(v1, v2);
        if (this->events_enabled()) {
            EventData event(EventType::ON_ADD_SIMPLEX, eid, 1, "Edge added");
            auto label_opt = this->get_label(eid);
            const LabelType* label_ptr = label_opt.has_value() ? &label_opt.value() : nullptr;
            this->event_system_.trigger_event(event, label_ptr);
        }
        return eid;
    }

    /**
     * @brief Add a triangle and trigger event
     */
    SimplexID add_triangle(VertexID v1, VertexID v2, VertexID v3) {
        SimplexID tid = SimplicialComplexLabeled<LabelType>::add_triangle(v1, v2, v3);
        if (this->events_enabled()) {
            EventData event(EventType::ON_ADD_SIMPLEX, tid, 2, "Triangle added");
            auto label_opt = this->get_label(tid);
            const LabelType* label_ptr = label_opt.has_value() ? &label_opt.value() : nullptr;
            this->event_system_.trigger_event(event, label_ptr);
        }
        return tid;
    }

    /**
     * @brief Add a k-simplex and trigger event
     */
    SimplexID add_simplex(const std::vector<VertexID>& vertices) {
        SimplexID sid = SimplicialComplexLabeled<LabelType>::add_simplex(vertices);
        if (this->events_enabled()) {
            size_t dim = vertices.size() - 1;
            EventData event(EventType::ON_ADD_SIMPLEX, sid, dim, "Simplex added");
            auto label_opt = this->get_label(sid);
            const LabelType* label_ptr = label_opt.has_value() ? &label_opt.value() : nullptr;
            this->event_system_.trigger_event(event, label_ptr);
        }
        return sid;
    }

    /**
     * @brief Remove a simplex and trigger event
     */
    bool remove_simplex(SimplexID simplex_id, bool cascade = false) {
        // Get label before removal
        auto label_opt = this->get_label(simplex_id);
        const LabelType* label_ptr = label_opt.has_value() ? &label_opt.value() : nullptr;

        bool removed = SimplicialComplexLabeled<LabelType>::remove_simplex(simplex_id, cascade);

        if (removed && this->events_enabled()) {
            EventData event(EventType::ON_REMOVE_SIMPLEX, simplex_id, 0, "Simplex removed");
            this->event_system_.trigger_event(event, label_ptr);
        }

        return removed;
    }

    /**
     * @brief Set label and trigger event
     */
    void set_label(SimplexID simplex_id, const LabelType& label) {
        bool had_label = this->has_label(simplex_id);
        SimplicialComplexLabeled<LabelType>::set_label(simplex_id, label);

        if (this->events_enabled()) {
            LabelChangeEvent event(simplex_id, 0, !had_label, true);
            this->event_system_.trigger_event(event, &label);
        }
    }

    /**
     * @brief Remove label and trigger event
     */
    void remove_label(SimplexID simplex_id) {
        bool had_label = this->has_label(simplex_id);
        SimplicialComplexLabeled<LabelType>::remove_label(simplex_id);

        if (had_label && this->events_enabled()) {
            LabelChangeEvent event(simplex_id, 0, false, false);
            this->event_system_.trigger_event(event, nullptr);
        }
    }

    /**
     * @brief Notify batch topology change begin
     */
    void notify_topology_change_begin(const std::string& operation_name,
                                      const std::vector<SimplexID>& affected_simplices = {}) {
        if (this->events_enabled()) {
            TopologyChangeEvent event(operation_name, true, affected_simplices);
            this->event_system_.trigger_event(event, nullptr);
        }
    }

    /**
     * @brief Notify batch topology change end
     */
    void notify_topology_change_end(const std::string& operation_name,
                                    const std::vector<SimplexID>& affected_simplices = {}) {
        if (this->events_enabled()) {
            TopologyChangeEvent event(operation_name, false, affected_simplices);
            this->event_system_.trigger_event(event, nullptr);
        }
    }

    /**
     * @brief Notify refinement begin
     */
    void notify_refine_begin(SimplexID simplex_id, int level) {
        if (this->events_enabled()) {
            RefinementEvent event(simplex_id, level, 0, 0, true);
            auto label_opt = this->get_label(simplex_id);
            const LabelType* label_ptr = label_opt.has_value() ? &label_opt.value() : nullptr;
            this->event_system_.trigger_event(event, label_ptr);
        }
    }

    /**
     * @brief Notify refinement end
     */
    void notify_refine_end(SimplexID simplex_id, int level,
                           size_t new_simplices, size_t new_vertices) {
        if (this->events_enabled()) {
            RefinementEvent event(simplex_id, level, new_simplices, new_vertices, false);
            auto label_opt = this->get_label(simplex_id);
            const LabelType* label_ptr = label_opt.has_value() ? &label_opt.value() : nullptr;
            this->event_system_.trigger_event(event, label_ptr);
        }
    }

    /**
     * @brief Notify coarsening begin
     */
    void notify_coarsen_begin(SimplexID simplex_id, int level) {
        if (this->events_enabled()) {
            CoarseningEvent event(simplex_id, level, 0, 0, true);
            auto label_opt = this->get_label(simplex_id);
            const LabelType* label_ptr = label_opt.has_value() ? &label_opt.value() : nullptr;
            this->event_system_.trigger_event(event, label_ptr);
        }
    }

    /**
     * @brief Notify coarsening end
     */
    void notify_coarsen_end(SimplexID simplex_id, int level,
                            size_t removed_simplices, size_t removed_vertices) {
        if (this->events_enabled()) {
            CoarseningEvent event(simplex_id, level, removed_simplices, removed_vertices, false);
            auto label_opt = this->get_label(simplex_id);
            const LabelType* label_ptr = label_opt.has_value() ? &label_opt.value() : nullptr;
            this->event_system_.trigger_event(event, label_ptr);
        }
    }

    /**
     * @brief Notify glue operation
     */
    void notify_glue(SimplexID id1, SimplexID id2, const std::string& description) {
        if (this->events_enabled()) {
            GlueEvent event(id1, id2, description);
            this->event_system_.trigger_event(event, nullptr);
        }
    }

    /**
     * @brief Notify separate operation
     */
    void notify_separate(SimplexID id1, SimplexID id2, const std::string& description) {
        if (this->events_enabled()) {
            SeparateEvent event(id1, id2, description);
            this->event_system_.trigger_event(event, nullptr);
        }
    }

    /**
     * @brief Notify morph begin
     */
    void notify_morph_begin(const std::string& operation_name,
                            const std::vector<SimplexID>& affected_simplices = {}) {
        if (this->events_enabled()) {
            EventData event(EventType::ON_MORPH_BEGIN, 0, 0,
                           "Morph operation: " + operation_name);
            event.operation_name = operation_name;
            event.affected_simplices = affected_simplices;
            this->event_system_.trigger_event(event, nullptr);
        }
    }

    /**
     * @brief Notify morph end
     */
    void notify_morph_end(const std::string& operation_name,
                          const std::vector<SimplexID>& affected_simplices = {}) {
        if (this->events_enabled()) {
            EventData event(EventType::ON_MORPH_END, 0, 0,
                           "Morph operation completed: " + operation_name);
            event.operation_name = operation_name;
            event.affected_simplices = affected_simplices;
            this->event_system_.trigger_event(event, nullptr);
        }
    }
};

} // namespace cebu
