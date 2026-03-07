#include "cebu/narrative_context.h"
#include <sstream>
#include <cmath>
#include <algorithm>

namespace cebu {

// =============================================================================
// NarrativeContext Implementation
// =============================================================================

NarrativeContext NarrativeContext::combine(
    const std::vector<std::pair<NarrativeContext, double>>& contexts) {
    if (contexts.empty()) {
        return NarrativeContext();
    }

    double total_weight = 0.0;
    double s_sum = 0.0, l_sum = 0.0, e_sum = 0.0, d_sum = 0.0, t_sum = 0.0;

    for (const auto& [ctx, weight] : contexts) {
        total_weight += weight;
        s_sum += weight * ctx.surprisal;
        l_sum += weight * ctx.logical_deviation;
        e_sum += weight * ctx.emotional_intensity;
        d_sum += weight * ctx.dramatic_tension;
        t_sum += weight * ctx.temporal_distance;
    }

    if (total_weight > 0.0) {
        return NarrativeContext(
            s_sum / total_weight,
            l_sum / total_weight,
            e_sum / total_weight,
            d_sum / total_weight,
            t_sum / total_weight
        );
    }
    return NarrativeContext();
}

// =============================================================================
// NarrativePresets Implementation
// =============================================================================

NarrativeContext NarrativePresets::get_preset(DramaticEventType type) {
    switch (type) {
        case DramaticEventType::EXPOSITION:
            return NarrativeContext(0.2, 0.3, 0.3, 0.2, 1.0);

        case DramaticEventType::INCITING_INCIDENT:
            return NarrativeContext(0.7, 0.5, 0.6, 0.5, 0.9);

        case DramaticEventType::RISING_ACTION:
            return NarrativeContext(0.5, 0.4, 0.5, 0.6, 0.7);

        case DramaticEventType::PLOT_POINT_1:
            return plot_point(true);

        case DramaticEventType::MIDPOINT:
            return NarrativeContext(0.6, 0.5, 0.7, 0.7, 0.5);

        case DramaticEventType::PLOT_POINT_2:
            return plot_point(false);

        case DramaticEventType::CLIMAX:
            return climax();

        case DramaticEventType::FALLING_ACTION:
            return NarrativeContext(0.4, 0.4, 0.5, 0.4, 0.3);

        case DramaticEventType::RESOLUTION:
            return NarrativeContext(0.1, 0.1, 0.3, 0.1, 0.0);

        case DramaticEventType::TWIST:
            return twist();

        case DramaticEventType::REVELATION:
            return NarrativeContext(0.9, 0.3, 0.8, 0.6, 0.5);

        case DramaticEventType::CONFLICT:
            return conflict();

        case DramaticEventType::TRANSITION:
            return transition();

        case DramaticEventType::PAUSE:
            return NarrativeContext(0.1, 0.2, 0.2, 0.1, 0.5);

        case DramaticEventType::FLASHBACK:
            return flashback();

        case DramaticEventType::FLASHFORWARD:
            return NarrativeContext(0.6, 0.7, 0.5, 0.6, 0.5);

        case DramaticEventType::MONTAGE:
            return NarrativeContext(0.3, 0.4, 0.6, 0.5, 0.5);

        case DramaticEventType::TIME_SKIP:
            return time_skip();

        case DramaticEventType::CUSTOM:
            return custom();

        default:
            return NarrativeContext();
    }
}

NarrativeContext NarrativePresets::plot_point(bool is_first) {
    if (is_first) {
        // First plot point: major turning point early in story
        return NarrativeContext(0.7, 0.6, 0.7, 0.8, 0.8);
    } else {
        // Second plot point: lead-in to climax
        return NarrativeContext(0.8, 0.7, 0.8, 0.9, 0.3);
    }
}

NarrativeContext NarrativePresets::climax(double intensity) {
    intensity = std::clamp(intensity, 0.0, 1.0);
    return NarrativeContext(
        0.9 + 0.1 * intensity,  // Very high surprisal
        0.8 + 0.2 * intensity,  // High logical deviation (chaos)
        1.0,                     // Maximum emotion
        1.0,                     // Maximum tension
        0.0                      // At climax
    );
}

NarrativeContext NarrativePresets::twist(double magnitude) {
    magnitude = std::clamp(magnitude, 0.0, 1.0);
    return NarrativeContext(
        0.5 + 0.5 * magnitude,  // High surprisal
        0.6 + 0.4 * magnitude,  // Significant logic shift
        0.7 + 0.3 * magnitude,  // Emotional impact
        0.8,                     // High tension
        0.5                      // Can happen anywhere
    );
}

NarrativeContext NarrativePresets::conflict(double intensity) {
    intensity = std::clamp(intensity, 0.0, 1.0);
    return NarrativeContext(
        0.4 + 0.3 * intensity,
        0.5 + 0.3 * intensity,
        0.6 + 0.4 * intensity,
        0.7 + 0.3 * intensity,
        0.5
    );
}

NarrativeContext NarrativePresets::flashback(double distance) {
    distance = std::clamp(distance, 0.0, 1.0);
    return NarrativeContext(
        0.3,  // Flashback reveals known info
        0.4 + 0.2 * distance,  // May have different logic
        0.5,
        0.4,
        distance  // Temporal distance
    );
}

NarrativeContext NarrativePresets::time_skip(double duration_factor) {
    duration_factor = std::clamp(duration_factor, 0.0, 1.0);
    return NarrativeContext(
        0.4 + 0.4 * duration_factor,  // Larger skips more surprising
        0.3 + 0.3 * duration_factor,
        0.5,
        0.3 + 0.3 * duration_factor,
        0.5
    );
}

NarrativeContext NarrativePresets::transition(bool abrupt) {
    if (abrupt) {
        return NarrativeContext(0.5, 0.4, 0.4, 0.4, 0.5);
    } else {
        return NarrativeContext(0.2, 0.2, 0.3, 0.2, 0.5);
    }
}

NarrativeContext NarrativePresets::custom(
    double surprisal,
    double logical_deviation,
    double emotional_intensity,
    double dramatic_tension,
    double temporal_distance) {
    return NarrativeContext(
        std::clamp(surprisal, 0.0, 1.0),
        std::clamp(logical_deviation, 0.0, 1.0),
        std::clamp(emotional_intensity, 0.0, 1.0),
        std::clamp(dramatic_tension, 0.0, 1.0),
        std::clamp(temporal_distance, 0.0, 1.0)
    );
}

// =============================================================================
// ThreeActStructure Implementation
// =============================================================================

ThreeActStructure::ThreeActStructure(double climax_position)
    : climax_position_(std::clamp(climax_position, 0.5, 0.9)) {}

std::vector<StoryBeat> ThreeActStructure::generate_beats(
    double start, double end) const {
    std::vector<StoryBeat> beats;
    double duration = end - start;

    // Act 1: Setup (0% - 25%)
    double act1_end = start + 0.25 * duration;
    beats.emplace_back(
        DramaticEventType::EXPOSITION,
        start,
        "Introduction of characters and setting",
        NarrativePresets::get_preset(DramaticEventType::EXPOSITION),
        0.1 * duration
    );
    beats.emplace_back(
        DramaticEventType::INCITING_INCIDENT,
        start + 0.15 * duration,
        "Inciting incident - problem appears",
        NarrativePresets::get_preset(DramaticEventType::INCITING_INCIDENT),
        0.05 * duration
    );
    beats.emplace_back(
        DramaticEventType::PLOT_POINT_1,
        act1_end,
        "First plot point - decision made",
        NarrativePresets::get_preset(DramaticEventType::PLOT_POINT_1),
        0.05 * duration
    );

    // Act 2: Confrontation (25% - 75%)
    double act2_start = act1_end;
    double act2_mid = start + 0.5 * duration;
    double act2_end = start + 0.75 * duration;

    beats.emplace_back(
        DramaticEventType::RISING_ACTION,
        act2_start + 0.1 * duration,
        "Rising action - challenges begin",
        NarrativePresets::get_preset(DramaticEventType::RISING_ACTION),
        0.15 * duration
    );
    beats.emplace_back(
        DramaticEventType::MIDPOINT,
        act2_mid,
        "Midpoint - stakes are raised",
        NarrativePresets::get_preset(DramaticEventType::MIDPOINT),
        0.05 * duration
    );
    beats.emplace_back(
        DramaticEventType::CONFLICT,
        act2_mid + 0.1 * duration,
        "Major conflict - hero struggles",
        NarrativePresets::get_preset(DramaticEventType::CONFLICT),
        0.1 * duration
    );
    beats.emplace_back(
        DramaticEventType::PLOT_POINT_2,
        act2_end,
        "Second plot point - everything seems lost",
        NarrativePresets::get_preset(DramaticEventType::PLOT_POINT_2),
        0.05 * duration
    );

    // Act 3: Resolution (75% - 100%)
    double act3_start = act2_end;

    beats.emplace_back(
        DramaticEventType::CLIMAX,
        start + climax_position_ * duration,
        "Climax - final confrontation",
        NarrativePresets::climax(),
        0.1 * duration
    );
    beats.emplace_back(
        DramaticEventType::FALLING_ACTION,
        start + (climax_position_ + 0.1) * duration,
        "Falling action - aftermath",
        NarrativePresets::get_preset(DramaticEventType::FALLING_ACTION),
        0.08 * duration
    );
    beats.emplace_back(
        DramaticEventType::RESOLUTION,
        end,
        "Resolution - new normal",
        NarrativePresets::get_preset(DramaticEventType::RESOLUTION),
        0.07 * duration
    );

    return beats;
}

// =============================================================================
// HerosJourneyStructure Implementation
// =============================================================================

std::vector<StoryBeat> HerosJourneyStructure::generate_beats(
    double start, double end) const {
    std::vector<StoryBeat> beats;
    double duration = end - start;

    // Ordinary World
    beats.emplace_back(
        DramaticEventType::EXPOSITION,
        start,
        "Ordinary World - hero's normal life",
        NarrativeContext(0.1, 0.1, 0.2, 0.1, 1.0),
        0.1 * duration
    );

    // Call to Adventure
    beats.emplace_back(
        DramaticEventType::INCITING_INCIDENT,
        start + 0.1 * duration,
        "Call to Adventure",
        NarrativeContext(0.7, 0.4, 0.6, 0.5, 0.9),
        0.05 * duration
    );

    // Refusal of Call
    beats.emplace_back(
        DramaticEventType::PAUSE,
        start + 0.15 * duration,
        "Refusal of Call - hesitation",
        NarrativeContext(0.2, 0.2, 0.3, 0.2, 0.8),
        0.05 * duration
    );

    // Meeting the Mentor
    beats.emplace_back(
        DramaticEventType::REVELATION,
        start + 0.2 * duration,
        "Meeting the Mentor",
        NarrativeContext(0.5, 0.3, 0.6, 0.4, 0.7),
        0.05 * duration
    );

    // Crossing the Threshold
    beats.emplace_back(
        DramaticEventType::TRANSITION,
        start + 0.25 * duration,
        "Crossing the Threshold",
        NarrativeContext(0.6, 0.5, 0.7, 0.6, 0.6),
        0.05 * duration
    );

    // Tests, Allies, Enemies
    beats.emplace_back(
        DramaticEventType::RISING_ACTION,
        start + 0.35 * duration,
        "Tests, Allies, Enemies",
        NarrativeContext(0.4, 0.4, 0.5, 0.5, 0.5),
        0.15 * duration
    );

    // Approach to Inmost Cave
    beats.emplace_back(
        DramaticEventType::CONFLICT,
        start + 0.55 * duration,
        "Approach to Inmost Cave",
        NarrativeContext(0.6, 0.5, 0.6, 0.7, 0.4),
        0.1 * duration
    );

    // The Ordeal
    beats.emplace_back(
        DramaticEventType::CLIMAX,
        start + 0.7 * duration,
        "The Ordeal - death and rebirth",
        NarrativeContext(0.8, 0.7, 0.9, 0.9, 0.3),
        0.1 * duration
    );

    // Reward
    beats.emplace_back(
        DramaticEventType::REVELATION,
        start + 0.8 * duration,
        "Reward - seizing the sword",
        NarrativeContext(0.5, 0.4, 0.7, 0.5, 0.3),
        0.05 * duration
    );

    // The Road Back
    beats.emplace_back(
        DramaticEventType::CONFLICT,
        start + 0.85 * duration,
        "The Road Back - not over yet",
        NarrativeContext(0.6, 0.5, 0.6, 0.6, 0.2),
        0.05 * duration
    );

    // Resurrection
    beats.emplace_back(
        DramaticEventType::TWIST,
        start + 0.92 * duration,
        "Resurrection - final test",
        NarrativeContext(0.7, 0.6, 0.8, 0.8, 0.15),
        0.05 * duration
    );

    // Return with Elixir
    beats.emplace_back(
        DramaticEventType::RESOLUTION,
        end,
        "Return with Elixir - transformed",
        NarrativeContext(0.1, 0.1, 0.4, 0.1, 0.0),
        0.08 * duration
    );

    return beats;
}

// =============================================================================
// SaveTheCatStructure Implementation
// =============================================================================

std::vector<StoryBeat> SaveTheCatStructure::generate_beats(
    double start, double end) const {
    std::vector<StoryBeat> beats;
    double duration = end - start;

    // Opening Image
    beats.emplace_back(
        DramaticEventType::EXPOSITION,
        start,
        "Opening Image",
        NarrativeContext(0.2, 0.2, 0.3, 0.2, 1.0),
        0.05 * duration
    );

    // Theme Stated
    beats.emplace_back(
        DramaticEventType::REVELATION,
        start + 0.05 * duration,
        "Theme Stated",
        NarrativeContext(0.3, 0.2, 0.4, 0.3, 0.95),
        0.03 * duration
    );

    // Set-Up
    beats.emplace_back(
        DramaticEventType::RISING_ACTION,
        start + 0.1 * duration,
        "Set-Up",
        NarrativeContext(0.2, 0.2, 0.3, 0.2, 0.9),
        0.1 * duration
    );

    // Catalyst
    beats.emplace_back(
        DramaticEventType::INCITING_INCIDENT,
        start + 0.2 * duration,
        "Catalyst",
        NarrativeContext(0.7, 0.5, 0.6, 0.5, 0.8),
        0.03 * duration
    );

    // Debate
    beats.emplace_back(
        DramaticEventType::PAUSE,
        start + 0.23 * duration,
        "Debate",
        NarrativeContext(0.3, 0.3, 0.4, 0.3, 0.75),
        0.07 * duration
    );

    // Break into Two
    beats.emplace_back(
        DramaticEventType::PLOT_POINT_1,
        start + 0.3 * duration,
        "Break into Two",
        NarrativeContext(0.6, 0.5, 0.6, 0.6, 0.7),
        0.05 * duration
    );

    // B Story
    beats.emplace_back(
        DramaticEventType::RISING_ACTION,
        start + 0.4 * duration,
        "B Story",
        NarrativeContext(0.3, 0.3, 0.5, 0.3, 0.6),
        0.1 * duration
    );

    // Fun and Games
    beats.emplace_back(
        DramaticEventType::RISING_ACTION,
        start + 0.55 * duration,
        "Fun and Games",
        NarrativeContext(0.4, 0.3, 0.5, 0.4, 0.5),
        0.15 * duration
    );

    // Midpoint
    beats.emplace_back(
        DramaticEventType::MIDPOINT,
        start + 0.7 * duration,
        "Midpoint",
        NarrativeContext(0.6, 0.5, 0.7, 0.7, 0.4),
        0.05 * duration
    );

    // Bad Guys Close In
    beats.emplace_back(
        DramaticEventType::CONFLICT,
        start + 0.75 * duration,
        "Bad Guys Close In",
        NarrativeContext(0.7, 0.6, 0.7, 0.8, 0.35),
        0.1 * duration
    );

    // All Is Lost
    beats.emplace_back(
        DramaticEventType::PLOT_POINT_2,
        start + 0.85 * duration,
        "All Is Lost",
        NarrativeContext(0.8, 0.7, 0.8, 0.9, 0.2),
        0.05 * duration
    );

    // Dark Night of the Soul
    beats.emplace_back(
        DramaticEventType::PAUSE,
        start + 0.9 * duration,
        "Dark Night of the Soul",
        NarrativeContext(0.2, 0.2, 0.3, 0.2, 0.15),
        0.03 * duration
    );

    // Break into Three
    beats.emplace_back(
        DramaticEventType::TRANSITION,
        start + 0.93 * duration,
        "Break into Three",
        NarrativeContext(0.7, 0.6, 0.8, 0.8, 0.12),
        0.03 * duration
    );

    // Finale
    beats.emplace_back(
        DramaticEventType::CLIMAX,
        start + 0.96 * duration,
        "Finale",
        NarrativeContext(0.9, 0.8, 0.9, 1.0, 0.06),
        0.03 * duration
    );

    // Final Image
    beats.emplace_back(
        DramaticEventType::RESOLUTION,
        end,
        "Final Image",
        NarrativeContext(0.1, 0.1, 0.4, 0.1, 0.0),
        0.04 * duration
    );

    return beats;
}

// =============================================================================
// NarrativeAnalyzer Implementation
// =============================================================================

NarrativeContext NarrativeAnalyzer::analyze(
    double current_time,
    const std::vector<StoryBeat>& past_beats,
    const std::vector<StoryBeat>& upcoming_beats) const {

    // Default context
    NarrativeContext result;

    // Analyze past beats for cumulative effect
    if (!past_beats.empty()) {
        double avg_surprisal = 0.0;
        double avg_logic = 0.0;
        double avg_emotion = 0.0;
        double avg_tension = 0.0;

        for (const auto& beat : past_beats) {
            const auto& ctx = beat.context;
            avg_surprisal += ctx.surprisal;
            avg_logic += ctx.logical_deviation;
            avg_emotion += ctx.emotional_intensity;
            avg_tension += ctx.dramatic_tension;
        }

        double n = past_beats.size();
        result.surprisal = avg_surprisal / n;
        result.logical_deviation = avg_logic / n;
        result.emotional_intensity = avg_emotion / n;
        result.dramatic_tension = avg_tension / n;
    }

    // Analyze upcoming beats for anticipation
    if (!upcoming_beats.empty()) {
        const auto& next = upcoming_beats[0];
        double distance = next.timestamp - current_time;

        // Blend past and future
        double blend_factor = std::exp(-distance);  // Closer = more influence

        result.surprisal = (1.0 - blend_factor) * result.surprisal +
                          blend_factor * next.context.surprisal;
        result.logical_deviation = (1.0 - blend_factor) * result.logical_deviation +
                               blend_factor * next.context.logical_deviation;
        result.emotional_intensity = (1.0 - blend_factor) * result.emotional_intensity +
                                   blend_factor * next.context.emotional_intensity;
        result.dramatic_tension = (1.0 - blend_factor) * result.dramatic_tension +
                                blend_factor * next.context.dramatic_tension;

        // Set temporal distance
        result.temporal_distance = std::clamp(distance / 1.0, 0.0, 1.0);
    }

    return result;
}

double NarrativeAnalyzer::calculate_tension(
    double current_time,
    const std::vector<StoryBeat>& beats) const {

    double tension = 0.0;
    double weight_sum = 0.0;

    for (const auto& beat : beats) {
        double distance = std::abs(beat.timestamp - current_time);
        double weight = std::exp(-distance * 5.0);  // Decay with distance
        tension += weight * beat.context.dramatic_tension;
        weight_sum += weight;
    }

    return weight_sum > 0.0 ? tension / weight_sum : 0.0;
}

bool NarrativeAnalyzer::is_approaching_climax(
    double current_time,
    const std::vector<StoryBeat>& beats,
    double window) const {

    for (const auto& beat : beats) {
        if (beat.type == DramaticEventType::CLIMAX) {
            double distance = beat.timestamp - current_time;
            return distance > 0.0 && distance <= window;
        }
    }
    return false;
}

std::optional<DramaticEventType> NarrativeAnalyzer::get_next_event_type(
    double current_time,
    const std::vector<StoryBeat>& beats) const {

    for (const auto& beat : beats) {
        if (beat.timestamp > current_time) {
            return beat.type;
        }
    }
    return std::nullopt;
}

double NarrativeAnalyzer::calculate_pacing(
    double current_time,
    const std::vector<StoryBeat>& beats,
    double window) const {

    auto recent = get_recent_beats(current_time, beats, window);
    if (recent.empty()) {
        return 0.0;
    }

    // Count events per unit time
    double time_span = window;
    double count = recent.size();
    return std::clamp(count / time_span, 0.0, 10.0);
}

double NarrativeAnalyzer::distance_to_next_beat(
    double current_time,
    const std::vector<StoryBeat>& beats) const {

    double min_distance = std::numeric_limits<double>::max();
    for (const auto& beat : beats) {
        if (beat.timestamp > current_time) {
            double distance = beat.timestamp - current_time;
            min_distance = std::min(min_distance, distance);
        }
    }
    return min_distance;
}

std::vector<StoryBeat> NarrativeAnalyzer::get_recent_beats(
    double current_time,
    const std::vector<StoryBeat>& beats,
    double window) const {

    std::vector<StoryBeat> result;
    for (const auto& beat : beats) {
        double distance = std::abs(beat.timestamp - current_time);
        if (distance <= window) {
            result.push_back(beat);
        }
    }
    return result;
}

} // namespace cebu
