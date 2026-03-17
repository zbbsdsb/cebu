#ifndef CEBU_STORY_EVENT_H
#define CEBU_STORY_EVENT_H

#include "cebu/absurdity.h"
#include "cebu/narrative_context.h"
#include "cebu/simplicial_complex.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace cebu {

/// Unique identifier for story events
using EventID = uint64_t;

/// Represents a single narrative event affecting the simplicial complex
struct StoryEvent {
    EventID id;                           ///< Unique event identifier
    std::string description;              ///< Human-readable description
    double timestamp;                     ///< Event time on timeline
    std::vector<SimplexID> affected_simplices;  ///< Simplices influenced by this event
    AbsurdityContext impact;              ///< Context driving absurdity evolution

    StoryEvent() : id(0), timestamp(0.0) {}

    StoryEvent(EventID _id, const std::string& _desc, double _time,
               const std::vector<SimplexID>& _simplices,
               const AbsurdityContext& _impact)
        : id(_id), description(_desc), timestamp(_time),
          affected_simplices(_simplices), impact(_impact) {}
};

/// Manages a collection of story events with time-based queries
class StoryEventSystem {
public:
    StoryEventSystem() = default;

    /// Add a new event to the timeline
    /// @throws std::invalid_argument if timestamp is negative
    EventID add_event(const std::string& description, double timestamp,
                     const std::vector<SimplexID>& simplices,
                     const AbsurdityContext& impact) {
        if (timestamp < 0.0) {
            throw std::invalid_argument("Event timestamp cannot be negative");
        }

        EventID id = next_event_id_++;
        events_.emplace(id, StoryEvent(id, description, timestamp, simplices, impact));
        return id;
    }

    /// Get all events within a time range [start, end]
    /// @returns Sorted vector of events by timestamp
    std::vector<StoryEvent> get_events_in_range(double start, double end) const {
        if (start > end) {
            throw std::invalid_argument("Start time must be <= end time");
        }

        std::vector<StoryEvent> result;
        for (const auto& pair : events_) {
            if (pair.second.timestamp >= start && pair.second.timestamp <= end) {
                result.push_back(pair.second);
            }
        }

        // Sort by timestamp
        std::sort(result.begin(), result.end(),
                 [](const StoryEvent& a, const StoryEvent& b) {
                     return a.timestamp < b.timestamp;
                 });

        return result;
    }

    /// Get event by ID
    /// @throws std::out_of_range if event not found
    const StoryEvent& get_event(EventID id) const {
        auto it = events_.find(id);
        if (it == events_.end()) {
            throw std::out_of_range("Event not found: " + std::to_string(id));
        }
        return it->second;
    }

    /// Remove an event from the system
    /// @throws std::out_of_range if event not found
    void remove_event(EventID id) {
        auto it = events_.find(id);
        if (it == events_.end()) {
            throw std::out_of_range("Event not found: " + std::to_string(id));
        }
        events_.erase(it);
    }

    /// Get total number of events
    size_t event_count() const {
        return events_.size();
    }

    /// Get all events (unsorted)
    const std::unordered_map<EventID, StoryEvent>& get_all_events() const {
        return events_;
    }

    /// Clear all events
    void clear() {
        events_.clear();
        next_event_id_ = 0;
    }

private:
    std::unordered_map<EventID, StoryEvent> events_;
    EventID next_event_id_ = 0;
};

} // namespace cebu

#endif // CEBU_STORY_EVENT_H
