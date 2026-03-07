#include "cebu/narrative_context.h"
#include "cebu/absurdity.h"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace cebu;

void test_narrative_context_basics() {
    std::cout << "Test 1: NarrativeContext basics... ";

    NarrativeContext ctx(0.7, 0.6, 0.8, 0.9, 0.5);

    assert(std::abs(ctx.surprisal - 0.7) < 1e-9);
    assert(std::abs(ctx.logical_deviation - 0.6) < 1e-9);
    assert(std::abs(ctx.emotional_intensity - 0.8) < 1e-9);
    assert(std::abs(ctx.dramatic_tension - 0.9) < 1e-9);
    assert(std::abs(ctx.temporal_distance - 0.5) < 1e-9);

    std::cout << "PASSED\n";
}

void test_driving_force() {
    std::cout << "Test 2: Driving force calculation... ";

    NarrativeContext ctx(0.8, 0.6, 0.5, 0.7, 0.5);
    double force = ctx.get_driving_force();

    // kappa * S + eta * L = 0.5 * 0.8 + 0.3 * 0.6 = 0.4 + 0.18 = 0.58
    assert(std::abs(force - 0.58) < 1e-9);

    std::cout << "PASSED\n";
}

void test_volatility() {
    std::cout << "Test 3: Volatility calculation... ";

    NarrativeContext ctx(0.5, 0.5, 0.8, 0.7, 0.2);
    double vol = ctx.get_volatility();

    // 0.1 + 0.2 * 0.8 + 0.3 * (1.0 - 0.2) = 0.1 + 0.16 + 0.24 = 0.5
    assert(std::abs(vol - 0.5) < 1e-9);

    std::cout << "PASSED\n";
}

void test_context_combination() {
    std::cout << "Test 4: Context combination... ";

    NarrativeContext ctx1(1.0, 0.0, 1.0, 0.0, 0.0);
    NarrativeContext ctx2(0.0, 1.0, 0.0, 1.0, 1.0);

    auto combined = NarrativeContext::combine({{ctx1, 1.0}, {ctx2, 1.0}});

    assert(std::abs(combined.surprisal - 0.5) < 1e-9);
    assert(std::abs(combined.logical_deviation - 0.5) < 1e-9);
    assert(std::abs(combined.emotional_intensity - 0.5) < 1e-9);
    assert(std::abs(combined.dramatic_tension - 0.5) < 1e-9);
    assert(std::abs(combined.temporal_distance - 0.5) < 1e-9);

    std::cout << "PASSED\n";
}

void test_narrative_presets() {
    std::cout << "Test 5: Narrative presets... ";

    auto exposition = NarrativePresets::get_preset(DramaticEventType::EXPOSITION);
    assert(exposition.surprisal < 0.5);  // Low surprisal

    auto climax = NarrativePresets::get_preset(DramaticEventType::CLIMAX);
    assert(climax.surprisal > 0.8);  // High surprisal
    assert(climax.dramatic_tension > 0.9);  // Max tension
    assert(climax.temporal_distance < 0.1);  // At climax

    auto twist = NarrativePresets::twist(0.8);
    assert(twist.surprisal > 0.8);  // High surprisal

    auto resolution = NarrativePresets::get_preset(DramaticEventType::RESOLUTION);
    assert(resolution.dramatic_tension < 0.2);  // Low tension

    std::cout << "PASSED\n";
}

void test_custom_context() {
    std::cout << "Test 6: Custom context... ";

    auto ctx = NarrativePresets::custom(0.9, 0.8, 0.7, 0.6, 0.5);

    assert(std::abs(ctx.surprisal - 0.9) < 1e-9);
    assert(std::abs(ctx.logical_deviation - 0.8) < 1e-9);
    assert(std::abs(ctx.emotional_intensity - 0.7) < 1e-9);
    assert(std::abs(ctx.dramatic_tension - 0.6) < 1e-9);
    assert(std::abs(ctx.temporal_distance - 0.5) < 1e-9);

    // Test clamping
    auto clamped = NarrativePresets::custom(1.5, -0.5, 2.0, -1.0, 1.5);
    assert(clamped.surprisal <= 1.0 && clamped.surprisal >= 0.0);
    assert(clamped.logical_deviation <= 1.0 && clamped.logical_deviation >= 0.0);

    std::cout << "PASSED\n";
}

void test_three_act_structure() {
    std::cout << "Test 7: Three-Act Structure... ";

    ThreeActStructure structure(0.75);
    auto beats = structure.generate_beats(0.0, 1.0);

    // Should have 9 beats (exposition, inciting incident, plot point 1,
    // rising action, midpoint, conflict, plot point 2, climax, falling action, resolution)
    assert(beats.size() == 9);

    // Check order
    assert(beats[0].type == DramaticEventType::EXPOSITION);
    assert(beats[1].type == DramaticEventType::INCITING_INCIDENT);
    assert(beats[2].type == DramaticEventType::PLOT_POINT_1);
    assert(beats[6].type == DramaticEventType::PLOT_POINT_2);
    assert(beats[7].type == DramaticEventType::CLIMAX);
    assert(beats[8].type == DramaticEventType::RESOLUTION);

    // Check climax position (should be at 0.75)
    bool climax_found = false;
    for (const auto& beat : beats) {
        if (beat.type == DramaticEventType::CLIMAX) {
            assert(std::abs(beat.timestamp - 0.75) < 0.01);
            climax_found = true;
        }
    }
    assert(climax_found);

    std::cout << "PASSED\n";
}

void test_heroes_journey_structure() {
    std::cout << "Test 8: Hero's Journey Structure... ";

    HerosJourneyStructure structure;
    auto beats = structure.generate_beats(0.0, 1.0);

    // Hero's Journey has 12 beats
    assert(beats.size() == 12);

    // Check some key beats
    bool ordinary_world = false;
    bool ordeal = false;
    bool return_with_elixir = false;

    for (const auto& beat : beats) {
        if (beat.description == "Ordinary World - hero's normal life") {
            ordinary_world = true;
        }
        if (beat.description == "The Ordeal - death and rebirth") {
            ordeal = true;
            assert(beat.context.surprisal > 0.7);
        }
        if (beat.description == "Return with Elixir - transformed") {
            return_with_elixir = true;
        }
    }

    assert(ordinary_world);
    assert(ordeal);
    assert(return_with_elixir);

    std::cout << "PASSED\n";
}

void test_save_the_cat_structure() {
    std::cout << "Test 9: Save the Cat Structure... ";

    SaveTheCatStructure structure;
    auto beats = structure.generate_beats(0.0, 1.0);

    // Save the Cat has 15 beats
    assert(beats.size() == 15);

    // Check key beats
    bool opening_image = false;
    bool catalyst = false;
    bool midpoint = false;
    bool all_is_lost = false;
    bool finale = false;

    for (const auto& beat : beats) {
        if (beat.description == "Opening Image") {
            opening_image = true;
        }
        if (beat.description == "Catalyst") {
            catalyst = true;
        }
        if (beat.description == "Midpoint") {
            midpoint = true;
        }
        if (beat.description == "All Is Lost") {
            all_is_lost = true;
            assert(all_is_lost);
            assert(beat.context.surprisal > 0.7);
        }
        if (beat.description == "Finale") {
            finale = true;
            assert(beat.context.dramatic_tension > 0.9);
        }
    }

    assert(opening_image);
    assert(catalyst);
    assert(midpoint);
    assert(all_is_lost);
    assert(finale);

    std::cout << "PASSED\n";
}

void test_custom_structure() {
    std::cout << "Test 10: Custom Structure... ";

    CustomNarrativeStructure structure;
    structure.add_beat(StoryBeat(
        DramaticEventType::EXPOSITION,
        0.0,
        "Custom opening",
        NarrativeContext(0.2, 0.2, 0.3, 0.2, 1.0),
        0.1
    ));
    structure.add_beat(StoryBeat(
        DramaticEventType::CLIMAX,
        0.5,
        "Custom climax",
        NarrativeContext(0.9, 0.8, 1.0, 1.0, 0.0),
        0.1
    ));

    auto beats = structure.generate_beats(0.0, 1.0);
    assert(beats.size() == 2);
    assert(beats[0].type == DramaticEventType::EXPOSITION);
    assert(beats[1].type == DramaticEventType::CLIMAX);

    std::cout << "PASSED\n";
}

void test_narrative_analyzer() {
    std::cout << "Test 11: Narrative Analyzer... ";

    NarrativeAnalyzer analyzer;

    std::vector<StoryBeat> past_beats = {
        StoryBeat(DramaticEventType::EXPOSITION, 0.0, "Start",
                 NarrativeContext(0.2, 0.2, 0.3, 0.2, 1.0), 0.1),
        StoryBeat(DramaticEventType::RISING_ACTION, 0.3, "Rising",
                 NarrativeContext(0.5, 0.4, 0.5, 0.6, 0.7), 0.2)
    };

    std::vector<StoryBeat> upcoming_beats = {
        StoryBeat(DramaticEventType::CLIMAX, 0.8, "Climax",
                 NarrativeContext(0.9, 0.8, 1.0, 1.0, 0.0), 0.1)
    };

    auto ctx = analyzer.analyze(0.5, past_beats, upcoming_beats);

    // Should blend past and future
    assert(ctx.surprisal > 0.2 && ctx.surprisal < 0.9);
    assert(ctx.dramatic_tension > 0.2 && ctx.dramatic_tension < 1.0);

    std::cout << "PASSED\n";
}

void test_tension_calculation() {
    std::cout << "Test 12: Tension calculation... ";

    NarrativeAnalyzer analyzer;

    std::vector<StoryBeat> beats = {
        StoryBeat(DramaticEventType::EXPOSITION, 0.0, "Start",
                 NarrativeContext(0.2, 0.2, 0.3, 0.2, 1.0), 0.1),
        StoryBeat(DramaticEventType::CLIMAX, 0.8, "Climax",
                 NarrativeContext(0.9, 0.8, 1.0, 1.0, 0.0), 0.1)
    };

    // At climax, tension should be high
    double tension_at_climax = analyzer.calculate_tension(0.8, beats);
    assert(tension_at_climax > 0.8);

    // At exposition, tension should be low
    double tension_at_start = analyzer.calculate_tension(0.0, beats);
    assert(tension_at_start < 0.5);

    std::cout << "PASSED\n";
}

void test_climax_detection() {
    std::cout << "Test 13: Climax detection... ";

    NarrativeAnalyzer analyzer;

    std::vector<StoryBeat> beats = {
        StoryBeat(DramaticEventType::RISING_ACTION, 0.0, "Rising",
                 NarrativeContext(0.5, 0.4, 0.5, 0.6, 0.7), 0.2),
        StoryBeat(DramaticEventType::CLIMAX, 0.8, "Climax",
                 NarrativeContext(0.9, 0.8, 1.0, 1.0, 0.0), 0.1)
    };

    // Not approaching when far
    assert(!analyzer.is_approaching_climax(0.0, beats, 0.1));

    // Approaching when close
    assert(analyzer.is_approaching_climax(0.75, beats, 0.1));

    // Past climax
    assert(!analyzer.is_approaching_climax(0.9, beats, 0.1));

    std::cout << "PASSED\n";
}

void test_next_event_type() {
    std::cout << "Test 14: Next event type detection... ";

    NarrativeAnalyzer analyzer;

    std::vector<StoryBeat> beats = {
        StoryBeat(DramaticEventType::EXPOSITION, 0.0, "Start",
                 NarrativeContext(0.2, 0.2, 0.3, 0.2, 1.0), 0.1),
        StoryBeat(DramaticEventType::CLIMAX, 0.5, "Climax",
                 NarrativeContext(0.9, 0.8, 1.0, 1.0, 0.0), 0.1),
        StoryBeat(DramaticEventType::RESOLUTION, 1.0, "End",
                 NarrativeContext(0.1, 0.1, 0.3, 0.1, 0.0), 0.1)
    };

    auto next_type = analyzer.get_next_event_type(0.3, beats);
    assert(next_type.has_value());
    assert(*next_type == DramaticEventType::CLIMAX);

    // No event after end
    auto no_next = analyzer.get_next_event_type(1.5, beats);
    assert(!no_next.has_value());

    std::cout << "PASSED\n";
}

void test_pacing_calculation() {
    std::cout << "Test 15: Pacing calculation... ";

    NarrativeAnalyzer analyzer;

    std::vector<StoryBeat> beats = {
        StoryBeat(DramaticEventType::EXPOSITION, 0.0, "Start",
                 NarrativeContext(0.2, 0.2, 0.3, 0.2, 1.0), 0.1),
        StoryBeat(DramaticEventType::RISING_ACTION, 0.1, "Rising",
                 NarrativeContext(0.5, 0.4, 0.5, 0.6, 0.7), 0.1),
        StoryBeat(DramaticEventType::CONFLICT, 0.2, "Conflict",
                 NarrativeContext(0.6, 0.5, 0.6, 0.7, 0.5), 0.1)
    };

    // High pacing (many events close together)
    double high_pacing = analyzer.calculate_pacing(0.15, beats, 0.2);
    assert(high_pacing > 5.0);

    std::cout << "PASSED\n";
}

void test_story_beat_creation() {
    std::cout << "Test 16: Story beat creation... ";

    StoryBeat beat(
        DramaticEventType::CLIMAX,
        0.75,
        "The final confrontation",
        NarrativeContext(0.9, 0.8, 1.0, 1.0, 0.0),
        0.1
    );

    assert(beat.type == DramaticEventType::CLIMAX);
    assert(std::abs(beat.timestamp - 0.75) < 1e-9);
    assert(beat.description == "The final confrontation");
    assert(beat.context.surprisal > 0.8);
    assert(std::abs(beat.duration - 0.1) < 1e-9);

    std::cout << "PASSED\n";
}

void test_context_to_string() {
    std::cout << "Test 17: Context to string... ";

    NarrativeContext ctx(0.5, 0.5, 0.5, 0.5, 0.5);
    ctx.event_type = "TestEvent";
    ctx.character_state = "Angry";

    std::string str = ctx.to_string();

    assert(str.find("S=0.5") != std::string::npos);
    assert(str.find("L=0.5") != std::string::npos);
    assert(str.find("E=0.5") != std::string::npos);
    assert(str.find("D=0.5") != std::string::npos);
    assert(str.find("T=0.5") != std::string::npos);
    assert(str.find("type=TestEvent") != std::string::npos);

    std::cout << "PASSED\n";
}

void test_climax_intensity() {
    std::cout << "Test 18: Climax intensity variation... ";

    auto mild_climax = NarrativePresets::climax(0.5);
    auto intense_climax = NarrativePresets::climax(1.0);

    assert(intense_climax.surprisal > mild_climax.surprisal);
    assert(intense_climax.logical_deviation > mild_climax.logical_deviation);
    assert(intense_climax.dramatic_tension == 1.0);
    assert(mild_climax.dramatic_tension == 1.0);

    std::cout << "PASSED\n";
}

void test_twist_magnitude() {
    std::cout << "Test 19: Twist magnitude variation... ";

    auto small_twist = NarrativePresets::twist(0.3);
    auto big_twist = NarrativePresets::twist(0.9);

    assert(big_twist.surprisal > small_twist.surprisal);
    assert(big_twist.logical_deviation > small_twist.logical_deviation);

    std::cout << "PASSED\n";
}

void test_conflict_intensity() {
    std::cout << "Test 20: Conflict intensity variation... ";

    auto low_conflict = NarrativePresets::conflict(0.3);
    auto high_conflict = NarrativePresets::conflict(0.9);

    assert(high_conflict.emotional_intensity > low_conflict.emotional_intensity);
    assert(high_conflict.dramatic_tension > low_conflict.dramatic_tension);

    std::cout << "PASSED\n";
}

int main() {
    std::cout << "=== Phase 9c: Narrative Context Tests ===\n\n";

    test_narrative_context_basics();
    test_driving_force();
    test_volatility();
    test_context_combination();
    test_narrative_presets();
    test_custom_context();
    test_three_act_structure();
    test_heroes_journey_structure();
    test_save_the_cat_structure();
    test_custom_structure();
    test_narrative_analyzer();
    test_tension_calculation();
    test_climax_detection();
    test_next_event_type();
    test_pacing_calculation();
    test_story_beat_creation();
    test_context_to_string();
    test_climax_intensity();
    test_twist_magnitude();
    test_conflict_intensity();

    std::cout << "\n=== All 20 tests PASSED! ===\n";
    return 0;
}
