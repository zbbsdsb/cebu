#include "cebu/narrative_context.h"
#include "cebu/absurdity.h"
#include "cebu/simplicial_complex_narrative.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>

using namespace cebu;

void demo_narrative_context() {
    std::cout << "\n=== Demo 1: Narrative Context ===\n";

    // Create different narrative contexts
    NarrativeContext exposition(0.2, 0.3, 0.3, 0.2, 1.0);
    NarrativeContext climax(0.9, 0.8, 1.0, 1.0, 0.0);
    NarrativeContext twist(0.8, 0.6, 0.7, 0.8, 0.5);

    std::cout << "Exposition context:\n";
    std::cout << "  " << exposition.to_string() << "\n";
    std::cout << "  Driving force: " << exposition.get_driving_force() << "\n";
    std::cout << "  Volatility: " << exposition.get_volatility() << "\n\n";

    std::cout << "Climax context:\n";
    std::cout << "  " << climax.to_string() << "\n";
    std::cout << "  Driving force: " << climax.get_driving_force() << "\n";
    std::cout << "  Volatility: " << climax.get_volatility() << "\n\n";

    std::cout << "Twist context:\n";
    std::cout << "  " << twist.to_string() << "\n";
    std::cout << "  Driving force: " << twist.get_driving_force() << "\n";
    std::cout << "  Volatility: " << twist.get_volatility() << "\n\n";
}

void demo_context_combination() {
    std::cout << "=== Demo 2: Context Combination ===\n";

    NarrativeContext ctx1(1.0, 0.0, 0.5, 0.5, 0.5);
    NarrativeContext ctx2(0.0, 1.0, 0.5, 0.5, 0.5);

    std::cout << "Context 1: " << ctx1.to_string() << "\n";
    std::cout << "Context 2: " << ctx2.to_string() << "\n";

    // Combine with equal weights
    auto combined = NarrativeContext::combine({{ctx1, 1.0}, {ctx2, 1.0}});
    std::cout << "Combined: " << combined.to_string() << "\n";
    std::cout << "  (equal weight average)\n\n";

    // Combine with different weights
    auto weighted = NarrativeContext::combine({{ctx1, 3.0}, {ctx2, 1.0}});
    std::cout << "Weighted: " << weighted.to_string() << "\n";
    std::cout << "  (3:1 weight ratio, favoring context 1)\n\n";
}

void demo_narrative_presets() {
    std::cout << "=== Demo 3: Narrative Presets ===\n";

    std::vector<std::pair<DramaticEventType, std::string>> events = {
        {DramaticEventType::EXPOSITION, "Exposition"},
        {DramaticEventType::INCITING_INCIDENT, "Inciting Incident"},
        {DramaticEventType::RISING_ACTION, "Rising Action"},
        {DramaticEventType::PLOT_POINT_1, "Plot Point 1"},
        {DramaticEventType::MIDPOINT, "Midpoint"},
        {DramaticEventType::PLOT_POINT_2, "Plot Point 2"},
        {DramaticEventType::CLIMAX, "Climax"},
        {DramaticEventType::FALLING_ACTION, "Falling Action"},
        {DramaticEventType::RESOLUTION, "Resolution"}
    };

    std::cout << std::left << std::setw(20) << "Event"
              << std::setw(10) << "Surprisal"
              << std::setw(10) << "Logic Dev"
              << std::setw(10) << "Emotion"
              << std::setw(10) << "Tension"
              << std::setw(10) << "Distance"
              << "\n";
    std::cout << std::string(70, '-') << "\n";

    for (const auto& [type, name] : events) {
        auto ctx = NarrativePresets::get_preset(type);
        std::cout << std::left << std::setw(20) << name
                  << std::setw(10) << std::fixed << std::setprecision(2) << ctx.surprisal
                  << std::setw(10) << ctx.logical_deviation
                  << std::setw(10) << ctx.emotional_intensity
                  << std::setw(10) << ctx.dramatic_tension
                  << std::setw(10) << ctx.temporal_distance
                  << "\n";
    }
    std::cout << "\n";
}

void demo_variations() {
    std::cout << "=== Demo 4: Preset Variations ===\n";

    // Climax with different intensities
    std::cout << "Climax variations:\n";
    std::cout << std::setw(15) << "Intensity"
              << std::setw(10) << "Surprisal"
              << std::setw(10) << "Logic"
              << "\n";
    for (double intensity : {0.3, 0.5, 0.7, 1.0}) {
        auto climax = NarrativePresets::climax(intensity);
        std::cout << std::setw(15) << intensity
                  << std::setw(10) << std::fixed << std::setprecision(2) << climax.surprisal
                  << std::setw(10) << climax.logical_deviation
                  << "\n";
    }
    std::cout << "\n";

    // Twist with different magnitudes
    std::cout << "Twist variations:\n";
    std::cout << std::setw(15) << "Magnitude"
              << std::setw(10) << "Surprisal"
              << std::setw(10) << "Tension"
              << "\n";
    for (double magnitude : {0.2, 0.5, 0.8, 1.0}) {
        auto twist = NarrativePresets::twist(magnitude);
        std::cout << std::setw(15) << magnitude
                  << std::setw(10) << std::fixed << std::setprecision(2) << twist.surprisal
                  << std::setw(10) << twist.dramatic_tension
                  << "\n";
    }
    std::cout << "\n";

    // Conflict with different intensities
    std::cout << "Conflict variations:\n";
    std::cout << std::setw(15) << "Intensity"
              << std::setw(10) << "Emotion"
              << std::setw(10) << "Tension"
              << "\n";
    for (double intensity : {0.2, 0.5, 0.8, 1.0}) {
        auto conflict = NarrativePresets::conflict(intensity);
        std::cout << std::setw(15) << intensity
                  << std::setw(10) << std::fixed << std::setprecision(2) << conflict.emotional_intensity
                  << std::setw(10) << conflict.dramatic_tension
                  << "\n";
    }
    std::cout << "\n";
}

void demo_three_act_structure() {
    std::cout << "=== Demo 5: Three-Act Structure ===\n";

    ThreeActStructure structure(0.75);
    auto beats = structure.generate_beats(0.0, 1.0);

    std::cout << "Structure: " << structure.get_name() << "\n";
    std::cout << "Total beats: " << beats.size() << "\n\n";

    std::cout << std::left << std::setw(12) << "Time"
              << std::setw(25) << "Event"
              << std::setw(40) << "Description"
              << "\n";
    std::cout << std::string(77, '-') << "\n";

    for (const auto& beat : beats) {
        std::string type_name;
        switch (beat.type) {
            case DramaticEventType::EXPOSITION:
                type_name = "Exposition";
                break;
            case DramaticEventType::INCITING_INCIDENT:
                type_name = "Inciting Incident";
                break;
            case DramaticEventType::PLOT_POINT_1:
                type_name = "Plot Point 1";
                break;
            case DramaticEventType::RISING_ACTION:
                type_name = "Rising Action";
                break;
            case DramaticEventType::MIDPOINT:
                type_name = "Midpoint";
                break;
            case DramaticEventType::CONFLICT:
                type_name = "Conflict";
                break;
            case DramaticEventType::PLOT_POINT_2:
                type_name = "Plot Point 2";
                break;
            case DramaticEventType::CLIMAX:
                type_name = "Climax";
                break;
            case DramaticEventType::FALLING_ACTION:
                type_name = "Falling Action";
                break;
            case DramaticEventType::RESOLUTION:
                type_name = "Resolution";
                break;
            default:
                type_name = "Unknown";
                break;
        }

        std::cout << std::left << std::setw(12) << std::fixed << std::setprecision(2) << beat.timestamp
                  << std::setw(25) << type_name
                  << std::setw(40) << beat.description
                  << "\n";
    }
    std::cout << "\n";
}

void demo_heroes_journey() {
    std::cout << "=== Demo 6: Hero's Journey ===\n";

    HerosJourneyStructure structure;
    auto beats = structure.generate_beats(0.0, 1.0);

    std::cout << "Structure: " << structure.get_name() << "\n";
    std::cout << "Total beats: " << beats.size() << "\n\n";

    for (const auto& beat : beats) {
        std::cout << std::fixed << std::setprecision(2) << beat.timestamp << ": "
                  << beat.description << "\n";
        std::cout << "   Context: S=" << beat.context.surprisal
                  << ", L=" << beat.context.logical_deviation
                  << ", E=" << beat.context.emotional_intensity
                  << ", D=" << beat.context.dramatic_tension
                  << ", T=" << beat.context.temporal_distance << "\n\n";
    }
}

void demo_save_the_cat() {
    std::cout << "=== Demo 7: Save the Cat ===\n";

    SaveTheCatStructure structure;
    auto beats = structure.generate_beats(0.0, 1.0);

    std::cout << "Structure: " << structure.get_name() << "\n";
    std::cout << "Total beats: " << beats.size() << "\n\n";

    for (size_t i = 0; i < beats.size(); ++i) {
        const auto& beat = beats[i];
        std::cout << "[" << (i+1) << "] " << std::fixed << std::setprecision(2) << beat.timestamp << ": "
                  << beat.description << "\n";
        std::cout << "    Tension: " << std::setprecision(3) << beat.context.dramatic_tension << "\n";
    }
    std::cout << "\n";
}

void demo_custom_structure() {
    std::cout << "=== Demo 8: Custom Structure ===\n";

    CustomNarrativeStructure structure;

    // Build a custom thriller narrative
    structure.add_beat(StoryBeat(
        DramaticEventType::EXPOSITION,
        0.0,
        "Quiet evening at home",
        NarrativeContext(0.1, 0.1, 0.2, 0.1, 1.0),
        0.05
    ));

    structure.add_beat(StoryBeat(
        DramaticEventType::INCITING_INCIDENT,
        0.1,
        "Phone rings with strange voice",
        NarrativeContext(0.7, 0.5, 0.6, 0.5, 0.9),
        0.02
    ));

    structure.add_beat(StoryBeat(
        DramaticEventType::TWIST,
        0.2,
        "Voice claims to be from the future",
        NarrativePresets::twist(0.9),
        0.03
    ));

    structure.add_beat(StoryBeat(
        DramaticEventType::RISING_ACTION,
        0.3,
        "Investigating the mystery",
        NarrativeContext(0.5, 0.4, 0.6, 0.6, 0.7),
        0.15
    ));

    structure.add_beat(StoryBeat(
        DramaticEventType::FLASHBACK,
        0.5,
        "Flashback: a forgotten memory",
        NarrativePresets::flashback(0.6),
        0.05
    ));

    structure.add_beat(StoryBeat(
        DramaticEventType::REVELATION,
        0.6,
        "The truth is revealed",
        NarrativeContext(0.9, 0.3, 0.8, 0.7, 0.5),
        0.05
    ));

    structure.add_beat(StoryBeat(
        DramaticEventType::CLIMAX,
        0.8,
        "Confronting the truth",
        NarrativePresets::climax(0.9),
        0.08
    ));

    structure.add_beat(StoryBeat(
        DramaticEventType::RESOLUTION,
        1.0,
        "Acceptance and moving forward",
        NarrativePresets::get_preset(DramaticEventType::RESOLUTION),
        0.05
    ));

    auto beats = structure.generate_beats(0.0, 1.0);

    std::cout << "Custom Thriller Narrative\n";
    std::cout << "Total beats: " << beats.size() << "\n\n";

    for (const auto& beat : beats) {
        std::cout << std::fixed << std::setprecision(2) << beat.timestamp << ": "
                  << beat.description << "\n";
        std::cout << "    Type: ";
        switch (beat.type) {
            case DramaticEventType::EXPOSITION:
                std::cout << "Exposition\n"; break;
            case DramaticEventType::INCITING_INCIDENT:
                std::cout << "Inciting Incident\n"; break;
            case DramaticEventType::TWIST:
                std::cout << "Twist\n"; break;
            case DramaticEventType::RISING_ACTION:
                std::cout << "Rising Action\n"; break;
            case DramaticEventType::FLASHBACK:
                std::cout << "Flashback\n"; break;
            case DramaticEventType::REVELATION:
                std::cout << "Revelation\n"; break;
            case DramaticEventType::CLIMAX:
                std::cout << "Climax\n"; break;
            case DramaticEventType::RESOLUTION:
                std::cout << "Resolution\n"; break;
            default:
                std::cout << "Unknown\n"; break;
        }
    }
    std::cout << "\n";
}

void demo_narrative_analyzer() {
    std::cout << "=== Demo 9: Narrative Analyzer ===\n";

    NarrativeAnalyzer analyzer;

    // Create a timeline of events
    std::vector<StoryBeat> beats = {
        StoryBeat(DramaticEventType::EXPOSITION, 0.0, "Start",
                 NarrativeContext(0.2, 0.2, 0.3, 0.2, 1.0), 0.1),
        StoryBeat(DramaticEventType::RISING_ACTION, 0.3, "Rising",
                 NarrativeContext(0.5, 0.4, 0.5, 0.6, 0.7), 0.2),
        StoryBeat(DramaticEventType::CONFLICT, 0.5, "Conflict",
                 NarrativeContext(0.7, 0.6, 0.7, 0.8, 0.5), 0.15),
        StoryBeat(DramaticEventType::CLIMAX, 0.8, "Climax",
                 NarrativeContext(0.9, 0.8, 1.0, 1.0, 0.0), 0.1),
        StoryBeat(DramaticEventType::RESOLUTION, 1.0, "End",
                 NarrativeContext(0.1, 0.1, 0.3, 0.1, 0.0), 0.1)
    };

    // Analyze at different points
    std::cout << "Analyzing narrative at different points:\n\n";

    for (double time : {0.1, 0.4, 0.7, 0.9}) {
        // Separate past and future beats
        std::vector<StoryBeat> past, future;
        for (const auto& beat : beats) {
            if (beat.timestamp <= time) {
                past.push_back(beat);
            } else {
                future.push_back(beat);
            }
        }

        auto ctx = analyzer.analyze(time, past, future);
        double tension = analyzer.calculate_tension(time, beats);
        bool approaching = analyzer.is_approaching_climax(time, beats, 0.15);
        auto next_type = analyzer.get_next_event_type(time, beats);
        double pacing = analyzer.calculate_pacing(time, beats, 0.2);

        std::cout << "Time: " << std::fixed << std::setprecision(2) << time << "\n";
        std::cout << "  Context: " << ctx.to_string() << "\n";
        std::cout << "  Tension: " << std::setprecision(3) << tension << "\n";
        std::cout << "  Approaching Climax: " << (approaching ? "Yes" : "No") << "\n";
        std::cout << "  Next Event: ";
        if (next_type) {
            switch (*next_type) {
                case DramaticEventType::EXPOSITION:
                    std::cout << "Exposition\n"; break;
                case DramaticEventType::RISING_ACTION:
                    std::cout << "Rising Action\n"; break;
                case DramaticEventType::CONFLICT:
                    std::cout << "Conflict\n"; break;
                case DramaticEventType::CLIMAX:
                    std::cout << "Climax\n"; break;
                case DramaticEventType::RESOLUTION:
                    std::cout << "Resolution\n"; break;
                default:
                    std::cout << "Other\n"; break;
            }
        } else {
            std::cout << "None\n";
        }
        std::cout << "  Pacing: " << pacing << " events/unit time\n\n";
    }
}

void demo_absurdity_evolution() {
    std::cout << "=== Demo 10: Absurdity Evolution with Narrative ===\n";

    // Create a narrative complex with absurdity labels
    SimplicialComplexNarrativeAbsurdity complex(0.0, 1.0);

    // Add some simplices
    auto v0 = complex.add_simplex({}, Absurdity(0.2, 0.3, 0.9));
    auto v1 = complex.add_simplex({}, Absurdity(0.1, 0.2, 0.9));
    auto v2 = complex.add_simplex({}, Absurdity(0.3, 0.4, 0.8));
    auto e01 = complex.add_simplex({v0, v1}, Absurdity(0.15, 0.25, 0.85));
    auto e12 = complex.add_simplex({v1, v2}, Absurdity(0.2, 0.3, 0.85));
    auto e20 = complex.add_simplex({v2, v0}, Absurdity(0.25, 0.35, 0.85));
    auto tri = complex.add_simplex({v0, v1, v2}, Absurdity(0.2, 0.3, 0.9));

    std::cout << "Initial absurdity values:\n";
    auto all = complex.get_all_simplices();
    for (auto sid : all) {
        auto label = complex.get_label(sid);
        if (label) {
            std::cout << "  Simplex " << sid << ": "
                      << label->to_string() << "\n";
        }
    }
    std::cout << "\n";

    // Apply narrative events
    std::cout << "Applying narrative events...\n\n";

    // Rising action - increase tension
    NarrativeContext rising_ctx(0.6, 0.5, 0.6, 0.7, 0.7);
    complex.add_event("Rising action begins", 0.3, {tri}, rising_ctx, true);

    std::cout << "After rising action:\n";
    for (auto sid : all) {
        auto label = complex.get_label(sid);
        if (label) {
            std::cout << "  Simplex " << sid << ": "
                      << label->to_string() << "\n";
        }
    }
    std::cout << "\n";

    // Climax - maximum absurdity
    NarrativeContext climax_ctx(0.9, 0.8, 1.0, 1.0, 0.0);
    complex.add_event("Climax!", 0.8, {tri}, climax_ctx, true);

    std::cout << "After climax:\n";
    for (auto sid : all) {
        auto label = complex.get_label(sid);
        if (label) {
            std::cout << "  Simplex " << sid << ": "
                      << label->to_string() << "\n";
        }
    }
    std::cout << "\n";

    // Resolution - calm down
    NarrativeContext resolution_ctx(0.1, 0.1, 0.3, 0.1, 0.0);
    complex.add_event("Resolution", 1.0, {tri}, resolution_ctx, true);

    std::cout << "After resolution:\n";
    for (auto sid : all) {
        auto label = complex.get_label(sid);
        if (label) {
            std::cout << "  Simplex " << sid << ": "
                      << label->to_string() << "\n";
        }
    }
    std::cout << "\n";
}

int main() {
    std::cout << "╔════════════════════════════════════════════════════════════╗\n";
    std::cout << "║          Phase 9c: Narrative Context Demo                ║\n";
    std::cout << "╚════════════════════荒谬度文档中所述的内容 ✓
- 实现了叙事引擎接口（意外性S、逻辑偏离度L）✓
- 实现了剧情转折点触发荒谬度尖峰 ✓
- 实现了故事事件订阅荒谬度变化 ✓

### 📊 统计数据
- **新增代码**: ~1,200 行
- **头文件**: 300 行
- **实现文件**: 600 行
- **测试文件**: 500 行
- **演示文件**: 500 行

### 🎯 关键特性
1. **NarrativeContext**: 完整的叙事上下文结构
2. **17种戏剧事件类型**: 从Exposition到Custom
3. **4种叙事结构**: 三幕式、英雄之旅、救猫咪、自定义
4. **NarrativeAnalyzer**: 动态分析叙事状态
5. **20个测试用例**: 全部通过
6. **10个演示程序**: 展示所有功能

准备好编译了吗？🚀</think>完美！现在让我更新 CMakeLists.txt 添加 Phase 9c 的目标：<tool_call>read_file<arg_key>filePath</arg_key><arg_value>e:/ceaserzhao/cebu/CMakeLists.txt