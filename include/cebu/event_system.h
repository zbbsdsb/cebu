#pragma once

#include "cebu/simplex.h"
#include <functional>
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <optional>

namespace cebu {

/**
 * @brief Event types for simplicial complex operations
 */
enum class EventType {
    ON_ADD_SIMPLEX,          ///< A simplex was added
    ON_REMOVE_SIMPLEX,       ///< A simplex was removed
    ON_LABEL_CHANGE,         ///< A label was changed
    ON_TOPOLOGY_CHANGE,      ///< Batch topology change began/ended
    ON_REFINE_BEGIN,         ///< Refinement operation began
    ON_REFINE_END,           ///< Refinement operation ended
    ON_COARSEN_BEGIN,        ///< Coarsening operation began
    ON_COARSEN_END,          ///< Coarsening operation ended
    ON_GLUE,                 ///< Two simplices were glued together
    ON_SEPARATE,             ///< Glued simplices were separated
    ON_MORPH_BEGIN,          ///< Morph operation began
    ON_MORPH_END             ///< Morph operation ended
};

/**
 * @brief Get string representation of event type
 */
inline const char* event_type_to_string(EventType type) {
    switch (type) {
        case EventType::ON_ADD_SIMPLEX: return "ON_ADD_SIMPLEX";
        case EventType::ON_REMOVE_SIMPLEX: return "ON_REMOVE_SIMPLEX";
        case EventType::ON_LABEL_CHANGE: return "ON_LABEL_CHANGE";
        case EventType::ON_TOPOLOGY_CHANGE: return "ON_TOPOLOGY_CHANGE";
        case EventType::ON_REFINE_BEGIN: return "ON_REFINE_BEGIN";
        case EventType::ON_REFINE_END: return "ON_REFINE_END";
        case EventType::ON_COARSEN_BEGIN: return "ON_COARSEN_BEGIN";
        case EventType::ON_COARSEN_END: return "ON_COARSEN_END";
        case EventType::ON_GLUE: return "ON_GLUE";
        case EventType::ON_SEPARATE: return "ON_SEPARATE";
        case EventType::ON_MORPH_BEGIN: return "ON_MORPH_BEGIN";
        case EventType::ON_MORPH_END: return "ON_MORPH_END";
        default: return "UNKNOWN";
    }
}

/**
 * @brief Event data structure
 */
struct EventData {
    EventType type;                                    ///< Event type
    SimplexID simplex_id;                              ///< Affected simplex ID
    size_t dimension;                                  ///< Dimension of simplex
    std::string description;                           ///< Human-readable description

    // Optional fields
    std::optional<SimplexID> related_id;              ///< Related simplex ID (for glue/separate)
    std::vector<SimplexID> affected_simplices;         ///< List of affected simplices (for batch operations)
    std::string operation_name;                        ///< Name of operation (for batch operations)

    EventData(EventType t, SimplexID sid = 0, size_t dim = 0, const std::string& desc = "")
        : type(t), simplex_id(sid), dimension(dim), description(desc) {}

    EventData() : type(EventType::ON_ADD_SIMPLEX), simplex_id(0), dimension(0) {}
};

/**
 * @brief Label change event data
 */
struct LabelChangeEvent : EventData {
    bool label_was_added;          ///< True if label was added, false if removed/changed
    bool label_exists_after;       ///< Whether label exists after the change

    LabelChangeEvent(SimplexID sid, size_t dim, bool added, bool exists)
        : EventData(EventType::ON_LABEL_CHANGE, sid, dim, "Label changed"),
          label_was_added(added), label_exists_after(exists) {}
};

/**
 * @brief Refinement event data
 */
struct RefinementEvent : EventData {
    int refinement_level;          ///< Current refinement level
    size_t new_simplices_count;    ///< Number of new simplices created
    size_t new_vertices_count;     ///< Number of new vertices created

    RefinementEvent(SimplexID sid, int level, size_t n_simplices, size_t n_vertices, bool begin)
        : EventData(begin ? EventType::ON_REFINE_BEGIN : EventType::ON_REFINE_END, sid, 0,
                   begin ? "Refinement began" : "Refinement ended"),
          refinement_level(level), new_simplices_count(n_simplices), new_vertices_count(n_vertices) {}
};

/**
 * @brief Coarsening event data
 */
struct CoarseningEvent : EventData {
    int refinement_level;          ///< Current refinement level
    size_t removed_simplices_count;///< Number of simplices removed
    size_t removed_vertices_count; ///< Number of vertices removed

    CoarseningEvent(SimplexID sid, int level, size_t n_simplices, size_t n_vertices, bool begin)
        : EventData(begin ? EventType::ON_COARSEN_BEGIN : EventType::ON_COARSEN_END, sid, 0,
                   begin ? "Coarsening began" : "Coarsening ended"),
          refinement_level(level), removed_simplices_count(n_simplices), removed_vertices_count(n_vertices) {}
};

/**
 * @brief Glue event data
 */
struct GlueEvent : EventData {
    GlueEvent(SimplexID sid1, SimplexID sid2, const std::string& desc)
        : EventData(EventType::ON_GLUE, sid1, 0, desc) {
        related_id = sid2;
    }
};

/**
 * @brief Separate event data
 */
struct SeparateEvent : EventData {
    SeparateEvent(SimplexID sid1, SimplexID sid2, const std::string& desc)
        : EventData(EventType::ON_SEPARATE, sid1, 0, desc) {
        related_id = sid2;
    }
};

/**
 * @brief Batch topology change event data
 */
struct TopologyChangeEvent : EventData {
    bool is_begin;                 ///< True if change began, false if ended

    TopologyChangeEvent(const std::string& op_name, bool begin,
                         const std::vector<SimplexID>& affected = {})
        : EventData(begin ? EventType::ON_TOPOLOGY_CHANGE : EventType::ON_TOPOLOGY_CHANGE,
                   0, 0, begin ? "Batch change began" : "Batch change ended"),
          is_begin(begin) {
        operation_name = op_name;
        affected_simplices = affected;
    }
};

/**
 * @brief Callback function type
 */
template<typename LabelType>
using EventCallback = std::function<void(const EventData&, const LabelType*)>;

/**
 * @brief Generic event callback (without label type)
 */
using GenericEventCallback = std::function<void(const EventData&)>;

/**
 * @brief Event system for simplicial complex
 *
 * Manages event registration, triggering, and callback execution.
 * Provides a publish-subscribe pattern for topology changes.
 */
template<typename LabelType>
class EventSystem {
public:
    using CallbackID = uint64_t;

    /**
     * @brief Constructor
     */
    EventSystem();

    /**
     * @brief Destructor
     */
    ~EventSystem();

    // Delete copy constructor and copy assignment
    EventSystem(const EventSystem&) = delete;
    EventSystem& operator=(const EventSystem&) = delete;

    // Allow move
    EventSystem(EventSystem&&) noexcept = default;
    EventSystem& operator=(EventSystem&&) noexcept = default;

    /**
     * @brief Register a callback for a specific event type
     * @param type Event type to subscribe to
     * @param callback Function to call when event occurs
     * @return Unique callback ID for unregistration
     */
    CallbackID register_callback(EventType type, EventCallback<LabelType> callback);

    /**
     * @brief Register a generic callback (without label access)
     * @param type Event type to subscribe to
     * @param callback Function to call when event occurs
     * @return Unique callback ID for unregistration
     */
    CallbackID register_callback(EventType type, GenericEventCallback callback);

    /**
     * @brief Unregister a callback
     * @param type Event type
     * @param callback_id Callback ID returned from register_callback
     * @return true if callback was removed, false if not found
     */
    bool unregister_callback(EventType type, CallbackID callback_id);

    /**
     * @brief Clear all callbacks for a specific event type
     * @param type Event type
     */
    void clear_callbacks(EventType type);

    /**
     * @brief Clear all callbacks for all event types
     */
    void clear_all_callbacks();

    /**
     * @brief Trigger an event
     * @param event Event data
     * @param label Optional label value for the affected simplex
     */
    void trigger_event(const EventData& event, const LabelType* label = nullptr);

    /**
     * @brief Trigger an event with automatic label lookup
     * @param event Event data
     * @param get_label_func Function to retrieve label by simplex ID
     */
    void trigger_event_with_label(const EventData& event,
                                   std::function<const LabelType*(SimplexID)> get_label_func);

    /**
     * @brief Get number of registered callbacks for an event type
     * @param type Event type
     * @return Number of callbacks
     */
    size_t callback_count(EventType type) const;

    /**
     * @brief Check if any callbacks are registered for an event type
     * @param type Event type
     * @return true if at least one callback is registered
     */
    bool has_callbacks(EventType type) const;

    /**
     * @brief Enable or disable event triggering
     * @param enabled true to enable, false to disable
     */
    void set_enabled(bool enabled);

    /**
     * @brief Check if event triggering is enabled
     * @return true if enabled
     */
    bool is_enabled() const { return enabled_; }

    /**
     * @brief Set event batching mode
     * @param batching true to enable batching
     *
     * When batching is enabled, events are queued and triggered
     * when flush_events() is called.
     */
    void set_batching(bool batching);

    /**
     * @brief Flush all queued events
     */
    void flush_events();

private:
    struct CallbackInfo {
        EventCallback<LabelType> typed_callback;
        GenericEventCallback generic_callback;
        CallbackID id;
    };

    std::unordered_map<EventType, std::vector<CallbackInfo>> callbacks_;
    std::unordered_map<EventType, CallbackID> next_callback_ids_;
    std::vector<std::pair<EventData, LabelType>> event_queue_;
    bool enabled_;
    bool batching_;
    CallbackID global_callback_id_counter_;
};

/**
 * @brief Convenience base class for event-emitting complexes
 *
 * Provides common event system functionality for simplicial complexes.
 */
template<typename LabelType>
class EventEmitter {
public:
    /**
     * @brief Get the event system
     */
    EventSystem<LabelType>& get_event_system() { return event_system_; }
    const EventSystem<LabelType>& get_event_system() const { return event_system_; }

    /**
     * @brief Register a callback
     */
    typename EventSystem<LabelType>::CallbackID register_callback(
        EventType type, EventCallback<LabelType> callback) {
        return event_system_.register_callback(type, callback);
    }

    /**
     * @brief Register a generic callback
     */
    typename EventSystem<LabelType>::CallbackID register_callback(
        EventType type, GenericEventCallback callback) {
        return event_system_.register_callback(type, callback);
    }

    /**
     * @brief Unregister a callback
     */
    bool unregister_callback(EventType type, typename EventSystem<LabelType>::CallbackID callback_id) {
        return event_system_.unregister_callback(type, callback_id);
    }

    /**
     * @brief Clear callbacks
     */
    void clear_callbacks(EventType type) {
        event_system_.clear_callbacks(type);
    }

    /**
     * @brief Enable/disable events
     */
    void set_events_enabled(bool enabled) {
        event_system_.set_enabled(enabled);
    }

    /**
     * @brief Check if events are enabled
     */
    bool events_enabled() const {
        return event_system_.is_enabled();
    }

protected:
    EventEmitter() = default;
    ~EventEmitter() = default;

    EventSystem<LabelType> event_system_;
};

} // namespace cebu
