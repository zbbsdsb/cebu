#include "cebu/event_system.h"
#include "cebu/absurdity.h"
#include <algorithm>
#include <string>

namespace cebu {

// ============================================================================
// EventSystem Implementation
// ============================================================================

template<typename LabelType>
EventSystem<LabelType>::EventSystem()
    : enabled_(true), batching_(false), global_callback_id_counter_(0) {}

template<typename LabelType>
EventSystem<LabelType>::~EventSystem() {
    clear_all_callbacks();
}

template<typename LabelType>
typename EventSystem<LabelType>::CallbackID
EventSystem<LabelType>::register_callback(EventType type, EventCallback<LabelType> callback) {
    CallbackID id = ++global_callback_id_counter_;

    CallbackInfo info;
    info.typed_callback = std::move(callback);
    info.generic_callback = nullptr;
    info.id = id;

    callbacks_[type].push_back(std::move(info));

    return id;
}

template<typename LabelType>
typename EventSystem<LabelType>::CallbackID
EventSystem<LabelType>::register_callback(EventType type, GenericEventCallback callback) {
    CallbackID id = ++global_callback_id_counter_;

    CallbackInfo info;
    info.typed_callback = nullptr;
    info.generic_callback = std::move(callback);
    info.id = id;

    callbacks_[type].push_back(std::move(info));

    return id;
}

template<typename LabelType>
bool EventSystem<LabelType>::unregister_callback(EventType type, CallbackID callback_id) {
    auto it = callbacks_.find(type);
    if (it == callbacks_.end()) {
        return false;
    }

    auto& callbacks = it->second;
    auto cb_it = std::find_if(callbacks.begin(), callbacks.end(),
                             [callback_id](const CallbackInfo& info) {
                                 return info.id == callback_id;
                             });

    if (cb_it != callbacks.end()) {
        callbacks.erase(cb_it);
        return true;
    }

    return false;
}

template<typename LabelType>
void EventSystem<LabelType>::clear_callbacks(EventType type) {
    callbacks_.erase(type);
    next_callback_ids_.erase(type);
}

template<typename LabelType>
void EventSystem<LabelType>::clear_all_callbacks() {
    callbacks_.clear();
    next_callback_ids_.clear();
    event_queue_.clear();
}

template<typename LabelType>
void EventSystem<LabelType>::trigger_event(const EventData& event, const LabelType* label) {
    if (!enabled_) {
        return;
    }

    if (batching_) {
        // Queue the event
        if (label) {
            event_queue_.emplace_back(event, *label);
        } else {
            // Use default-constructed label
            event_queue_.emplace_back(event, LabelType());
        }
        return;
    }

    auto it = callbacks_.find(event.type);
    if (it == callbacks_.end()) {
        return;
    }

    // Execute all callbacks for this event type
    for (const auto& info : it->second) {
        if (info.typed_callback) {
            info.typed_callback(event, label);
        } else if (info.generic_callback) {
            info.generic_callback(event);
        }
    }
}

template<typename LabelType>
void EventSystem<LabelType>::trigger_event_with_label(
    const EventData& event,
    std::function<const LabelType*(SimplexID)> get_label_func) {

    if (!enabled_) {
        return;
    }

    const LabelType* label = nullptr;
    if (get_label_func && event.simplex_id != 0) {
        label = get_label_func(event.simplex_id);
    }

    trigger_event(event, label);
}

template<typename LabelType>
size_t EventSystem<LabelType>::callback_count(EventType type) const {
    auto it = callbacks_.find(type);
    return it != callbacks_.end() ? it->second.size() : 0;
}

template<typename LabelType>
bool EventSystem<LabelType>::has_callbacks(EventType type) const {
    auto it = callbacks_.find(type);
    return it != callbacks_.end() && !it->second.empty();
}

template<typename LabelType>
void EventSystem<LabelType>::set_enabled(bool enabled) {
    enabled_ = enabled;
}

template<typename LabelType>
void EventSystem<LabelType>::set_batching(bool batching) {
    batching_ = batching;
}

template<typename LabelType>
void EventSystem<LabelType>::flush_events() {
    if (!batching_) {
        return;
    }

    // Process all queued events
    for (const auto& [event, label] : event_queue_) {
        auto it = callbacks_.find(event.type);
        if (it != callbacks_.end()) {
            const LabelType* label_ptr = &label;
            for (const auto& info : it->second) {
                if (info.typed_callback) {
                    info.typed_callback(event, label_ptr);
                } else if (info.generic_callback) {
                    info.generic_callback(event);
                }
            }
        }
    }

    event_queue_.clear();
}

// Explicit template instantiations for common label types
template class EventSystem<double>;
template class EventSystem<float>;
template class EventSystem<int>;
template class EventSystem<Absurdity>;
template class EventSystem<std::string>;

} // namespace cebu
