#pragma once

#include "cebu/absurdity.h"
#include <string>
#include <optional>
#include <vector>
#include <unordered_map>

namespace cebu {

/**
 * @brief Context for narrative-driven absurdity evolution
 *
 * This structure captures all external factors that influence absurdity:
 * - Surprisal (S): How unexpected the current situation is [0,1]
 * - Logical Deviation (L): How much this deviates from expected logic [0,1]
 * - Emotional Intensity: Current emotional state [0,1]
 * - Dramatic Tension: Narrative tension level [0,1]
 * - Temporal Distance: Distance from climax [0,1]
 */
struct NarrativeContext {
    double surprisal = 0.5;           ///< Unexpectedness of current event [0,1]
    double logical_deviation = 0.5;   ///< Deviation from logical consistency [0,1]
    double emotional_intensity = 0.5;  ///< Emotional state [0,1]
    double dramatic_tension = 0.5;    ///< Narrative tension [0,1]
    double temporal_distance = 0.5;   ///< Distance from story climax [0,1]

    // Optional metadata
    std::string event_type;           ///< Type of narrative event
    std::string character_state;      ///< Current character state
    std::optional<double> time_until_climax;  ///< Time until next climax

    NarrativeContext() = default;

    NarrativeContext(double s, double l, double e, double d, double t)
        : surprisal(s), logical_deviation(l),
          emotional_intensity(e), dramatic_tension(d),
          temporal_distance(t) {}

    // Combine contexts (weighted average)
    static NarrativeContext combine(
        const std::vector<std::pair<NarrativeContext, double>>& contexts);

    // Get effective driving force for absurdity evolution
    double get_driving_force() const {
        // SDE driving term: κ*S + η*L
        const double kappa = 0.5;  // Coupling to surprisal
        const double eta = 0.3;    // Coupling to logic
        return kappa * surprisal + eta * logical_deviation;
    }

    // Get volatility (noise intensity)
    double get_volatility() const {
        // Higher emotional intensity = more volatility
        // Closer to climax = more volatility
        return 0.1 + 0.2 * emotional_intensity + 0.3 * (1.0 - temporal_distance);
    }

    // String representation
    std::string to_string() const {
        std::ostringstream oss;
        oss << "NarrativeContext{S=" << surprisal
            << ", L=" << logical_deviation
            << ", E=" << emotional_intensity
            << ", D=" << dramatic_tension
            << ", T=" << temporal_distance;
        if (!event_type.empty()) {
            oss << ", type=" << event_type;
        }
        oss << "}";
        return oss.str();
    }
};

// Typedef for compatibility with existing code
using AbsurdityContext = NarrativeContext;

/**
 * @brief Dramatic event type with predefined context
 */
enum class DramaticEventType {
    EXPOSITION,           ///< Introduction of characters/setting
    INCITING_INCIDENT,    ///< Event that starts the story
    RISING_ACTION,        ///< Building tension
    PLOT_POINT_1,         ///< Major turning point
    MIDPOINT,             ///< Story midpoint
    PLOT_POINT_2,         ///< Second major turning point
    CLIMAX,               ///< Peak of the story
    FALLING_ACTION,       ///< Aftermath of climax
    RESOLUTION,           ///< Story conclusion
    TWIST,                ///< Unexpected plot twist
    REVELATION,           ///< Dramatic revelation
    CONFLICT,             ///< Character conflict
    TRANSITION,           ///< Scene transition
    PAUSE,                ///< Narrative pause
    FLASHBACK,            ///< Flashback scene
    FLASHFORWARD,         ///< Flashforward scene
    MONTAGE,              ///< Montage sequence
    TIME_SKIP,            ///< Jump forward in time
    CUSTOM                ///< User-defined event type
};

/**
 * @brief Predefined narrative context presets for dramatic events
 */
class NarrativePresets {
public:
    /// Get context for a dramatic event type
    static NarrativeContext get_preset(DramaticEventType type);

    /// Get context for plot point (first or second)
    static NarrativeContext plot_point(bool is_first);

    /// Get context for climax (with optional intensity)
    static NarrativeContext climax(double intensity = 1.0);

    /// Get context for twist (magnitude of unexpectedness)
    static NarrativeContext twist(double magnitude = 0.8);

    /// Get context for conflict (conflict intensity)
    static NarrativeContext conflict(double intensity = 0.7);

    /// Get context for flashback (distance from main timeline)
    static NarrativeContext flashback(double distance = 0.6);

    /// Get context for time skip (amount of time skipped)
    static NarrativeContext time_skip(double duration_factor = 0.8);

    /// Get context for transition (smooth vs abrupt)
    static NarrativeContext transition(bool abrupt = false);

    /// Custom context builder
    static NarrativeContext custom(
        double surprisal,
        double logical_deviation,
        double emotional_intensity = 0.5,
        double dramatic_tension = 0.5,
        double temporal_distance = 0.5);
};

/**
 * @brief Story beat in narrative structure
 */
struct StoryBeat {
    DramaticEventType type;
    double timestamp;
    std::string description;
    NarrativeContext context;
    double duration;  ///< How long this beat lasts

    StoryBeat(DramaticEventType t, double time, const std::string& desc,
              const NarrativeContext& ctx, double dur = 1.0)
        : type(t), timestamp(time), description(desc),
          context(ctx), duration(dur) {}
};

/**
 * @brief Narrative structure template (e.g., Three-Act, Hero's Journey)
 */
class NarrativeStructure {
public:
    virtual ~NarrativeStructure() = default;

    /// Get all story beats for this structure
    virtual std::vector<StoryBeat> generate_beats(double timeline_start, double timeline_end) const = 0;

    /// Get structure name
    virtual std::string get_name() const = 0;

protected:
    NarrativeStructure() = default;
};

/**
 * @brief Three-Act Structure (classic dramatic structure)
 */
class ThreeActStructure : public NarrativeStructure {
public:
    ThreeActStructure(double climax_position = 0.75);

    std::vector<StoryBeat> generate_beats(double start, double end) const override;
    std::string get_name() const override { return "Three-Act Structure"; }

    void set_climax_position(double pos) {
        climax_position_ = std::clamp(pos, 0.5, 0.9);
    }

private:
    double climax_position_;
};

/**
 * @brief Hero's Journey (monomyth)
 */
class HerosJourneyStructure : public NarrativeStructure {
public:
    std::vector<StoryBeat> generate_beats(double start, double end) const override;
    std::string get_name() const override { return "Hero's Journey"; }
};

/**
 * @brief Save the Cat (Blake Snyder's structure)
 */
class SaveTheCatStructure : public NarrativeStructure {
public:
    std::vector<StoryBeat> generate_beats(double start, double end) const override;
    std::string get_name() const override { return "Save the Cat"; }
};

/**
 * @brief Custom narrative structure from story beats
 */
class CustomNarrativeStructure : public NarrativeStructure {
public:
    explicit CustomNarrativeStructure(const std::vector<StoryBeat>& beats)
        : beats_(beats) {}

    std::vector<StoryBeat> generate_beats(double start, double end) const override {
        return beats_;  // Return the pre-defined beats
    }

    std::string get_name() const override { return "Custom Structure"; }

    /// Add a beat to the structure
    void add_beat(const StoryBeat& beat) {
        beats_.push_back(beat);
    }

    /// Clear all beats
    void clear() {
        beats_.clear();
    }

private:
    std::vector<StoryBeat> beats_;
};

/**
 * @brief Narrative analyzer for dynamic context generation
 *
 * Analyzes the current story state and generates appropriate contexts
 */
class NarrativeAnalyzer {
public:
    NarrativeAnalyzer() = default;

    /// Analyze current state and suggest context
    NarrativeContext analyze(
        double current_time,
        const std::vector<StoryBeat>& past_beats,
        const std::vector<StoryBeat>& upcoming_beats) const;

    /// Calculate narrative tension at a given time
    double calculate_tension(double current_time,
                            const std::vector<StoryBeat>& beats) const;

    /// Detect if we're approaching a climax
    bool is_approaching_climax(double current_time,
                              const std::vector<StoryBeat>& beats,
                              double window = 0.1) const;

    /// Get next dramatic event type
    std::optional<DramaticEventType> get_next_event_type(
        double current_time,
        const std::vector<StoryBeat>& beats) const;

    /// Calculate pacing (how fast events are happening)
    double calculate_pacing(double current_time,
                          const std::vector<StoryBeat>& beats,
                          double window = 0.2) const;

private:
    // Helper functions
    double distance_to_next_beat(double current_time,
                                const std::vector<StoryBeat>& beats) const;
    std::vector<StoryBeat> get_recent_beats(double current_time,
                                           const std::vector<StoryBeat>& beats,
                                           double window) const;
};

} // namespace cebu
