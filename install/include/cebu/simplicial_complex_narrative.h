#ifndef CEBU_SIMPLICIAL_COMPLEX_NARRATIVE_H
#define CEBU_SIMPLICIAL_COMPLEX_NARRATIVE_H

#include "cebu/simplicial_complex_labeled.h"
#include "cebu/story_event.h"
#include "cebu/timeline.h"
#include "cebu/absurdity.h"
#include "cebu/command_history.h"
#include <vector>
#include <algorithm>
#include <type_traits>
#include <memory>

// Forward declaration of serializer
namespace cebu {
    class JsonSerializer;
}

namespace cebu {

/**
 * @brief Narrative-driven simplicial complex with timeline and events
 *
 * This class extends the labeled simplicial complex with:
 * - Timeline management for narrative progression
 * - Story events that affect topology and labels
 * - Evolution of labels (especially absurdity) based on events
 * - Command history for undo/redo functionality
 * - Serialization support for saving/loading state
 *
 * @tparam LabelType The type of labels attached to simplices
 */
template<typename LabelType>
class SimplicialComplexNarrative : public SimplicialComplexLabeled<LabelType> {
public:
    using Base = SimplicialComplexLabeled<LabelType>;

    /// Constructor
    SimplicialComplexNarrative(
        double timeline_start = 0.0,
        double timeline_end = 1.0)
        : Base()
        , timeline_(timeline_start, timeline_end) {}

    /// Constructor with custom label system
    template<typename LabelSystemType>
    SimplicialComplexNarrative(
        std::unique_ptr<LabelSystemType> label_system,
        double timeline_start = 0.0,
        double timeline_end = 1.0)
        : Base(std::move(label_system))
        , timeline_(timeline_start, timeline_end) {}

    /// Get timeline
    Timeline& timeline() { return timeline_; }
    const Timeline& timeline() const { return timeline_; }

    /// Get event system
    StoryEventSystem& events() { return events_; }
    const StoryEventSystem& events() const { return events_; }

    /// Get command history for undo/redo
    CommandHistory& command_history() { return command_history_; }
    const CommandHistory& command_history() const { return command_history_; }

    /// Evolve complex to a specific timestamp
    /// Applies all events up to and including the given timestamp
    void evolve_to(double timestamp) {
        if (!timeline().contains(timestamp)) {
            throw std::invalid_argument(
                "Timestamp " + std::to_string(timestamp) +
                " is outside timeline bounds"
            );
        }

        // Get all events up to this timestamp
        double start = timeline().get_bounds().first;
        auto range_events = events().get_events_in_range(start, timestamp);

        // Apply each event
        for (const auto& event : range_events) {
            apply_event(event);
        }

        current_time_ = timestamp;
    }

    /// Apply a single event to the complex
    /// Updates labels on affected simplices based on event impact
    void apply_event(const StoryEvent& event) {
        // For each affected simplex, evolve its label
        for (SimplexID sid : event.affected_simplices) {
            auto label_opt = this->get_label(sid);
            if (label_opt) {
                LabelType updated = evolve_label(*label_opt, event.impact);
                this->set_label(sid, updated);
            }
        }
    }

    /// Add event and optionally apply it
    EventID add_event(const std::string& description, double timestamp,
                    const std::vector<SimplexID>& simplices,
                    const AbsurdityContext& impact,
                    bool apply_now = false) {
        EventID id = events().add_event(description, timestamp, simplices, impact);
        if (apply_now) {
            const StoryEvent& event = events().get_event(id);
            apply_event(event);
        }
        return id;
    }

    /// Get current simulation time
    double current_time() const { return current_time_; }

    /// Reset simulation to beginning of timeline
    void reset() {
        current_time_ = timeline().get_bounds().first;
        // Note: This does not undo label changes
        // For full reset, you would need to store initial labels
    }

    /// Save current state to string
    std::string serialize_state() const {
        return JsonSerializer::serialize_narrative(*this);
    }

    /// Load state from string (not implemented - requires JSON parser)
    void load_state(const std::string& state) {
        // Full implementation requires a JSON parser library
        // This is a placeholder for future enhancement
        throw std::runtime_error("load_state not yet implemented - requires JSON parser");
    }

    /// Create a snapshot at current time
    struct StateSnapshot {
        double time;
        std::string label_data;
    };

    StateSnapshot create_snapshot() const {
        StateSnapshot snapshot;
        snapshot.time = current_time_;
        snapshot.label_data = JsonSerializer::serialize_narrative(*this);
        return snapshot;
    }

    /// Restore from snapshot
    void restore_snapshot(const StateSnapshot& snapshot) {
        // Reset timeline
        current_time_ = snapshot.time;
        // Full restoration would require deserialization
        throw std::runtime_error("restore_snapshot not yet implemented - requires JSON parser");
    }

    /// Undo last command
    void undo() {
        command_history_.undo();
    }

    /// Redo last undone command
    void redo() {
        command_history_.redo();
    }

    /// Check if undo is available
    bool can_undo() const {
        return command_history_.can_undo();
    }

    /// Check if redo is available
    bool can_redo() const {
        return command_history_.can_redo();
    }

protected:
    /// Evolution hook for custom label types
    /// Override this to provide custom evolution logic
    virtual LabelType evolve_label(const LabelType& label,
                                 const AbsurdityContext& context) {
        // Default implementation uses update method if available
        // This works for Absurdity type
        if constexpr (std::is_same_v<LabelType, Absurdity>) {
            return label.update(context);
        }
        // For other label types, return unchanged
        return label;
    }

private:
    Timeline timeline_;
    StoryEventSystem events_;
    CommandHistory command_history_;
    double current_time_ = 0.0;
};

/// Convenience alias for double labels
using SimplicialComplexNarrativeDouble = SimplicialComplexNarrative<double>;

/// Convenience alias for Absurdity labels
using SimplicialComplexNarrativeAbsurdity = SimplicialComplexNarrative<Absurdity>;

} // namespace cebu

#endif // CEBU_SIMPLICIAL_COMPLEX_NARRATIVE_H
