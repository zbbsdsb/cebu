# Phase 9c: Narrative Context System

## Overview

Phase 9c implements a complete narrative context system, deeply integrating absurdity evolution with dramatic narrative structure. By providing key parameters such as **Surprisal (S)**, **Logic Deviation (L)**, **Emotional Intensity**, and **Dramatic Tension**, it drives the Phase 9a absurdity stochastic evolution equation (SDE).

## Core Concepts

### Narrative Context

Narrative context describes the dramatic state of a story at a specific moment, containing the following core dimensions:

- **Surprisal (S)**: `[0, 1]` - The degree of unexpectedness in the current situation
- **Logic Deviation (L)**: `[0, 1]` - The degree of deviation from logical consistency
- **Emotional Intensity (E)**: `[0, 1]` - Current emotional state
- **Dramatic Tension (T)**: `[0, 1]` - Narrative tension level
- **Time Distance (D)**: `[0, 1]` - Distance from the climax (0 = climax, 1 = beginning)

### Integration with Absurdity Evolution

```cpp
NarrativeContext ctx(0.8, 0.7, 0.9, 0.9, 0.0);

// Drive absurdity SDE equation
params.volatility = ctx.get_volatility();           // σ = σ₀ · (1 + E) · (1 + T)
params.coupling_strength = ctx.get_driving_force(); // κ·S + η·L

// Evolve absurdity
auto new_absurdity = evolution.step(current, params);
```

## API Reference

### `NarrativeContext`

#### Constructor

```cpp
NarrativeContext(
    double surprisal = 0.0,              // Surprisal S
    double logic_deviation = 0.0,        // Logic Deviation L
    double emotional_intensity = 0.0,    // Emotional Intensity E
    double dramatic_tension = 0.0,      // Dramatic Tension T
    double time_distance = 1.0          // Time Distance D
);
```

#### Core Methods

```cpp
// Get SDE driving term: κ·S + η·L
double get_driving_force(double kappa = 1.0, double eta = 1.0) const;

// Get volatility: σ₀ · (1 + E) · (1 + T)
double get_volatility(double base_volatility = 0.1) const;

// Fuse with another context
NarrativeContext fuse(const NarrativeContext& other, double weight = 0.5) const;

// Convert to string
std::string to_string() const;
```

#### Getter Methods

```cpp
double surprisal() const;           // Surprisal S
double logic_deviation() const;     // Logic Deviation L
double emotional_intensity() const; // Emotional Intensity E
double dramatic_tension() const;    // Dramatic Tension T
double time_distance() const;      // Time Distance D
```

---

### `DramaticEventType`

Dramatic event type enumeration, 17 types total:

#### Classic Events (9 types)

| Event Type | Enum | Description |
|-----------|------|-------------|
| Exposition | `EXPOSITION` | Setup: Introduce characters and setting |
| Inciting Incident | `INCITING_INCIDENT` | Inciting Incident: Disrupts equilibrium |
| Rising Action | `RISING_ACTION` | Rising Action: Escalating conflict |
| Plot Point 1 | `PLOT_POINT_1` | Plot Point 1: Enter Act 2 |
| Midpoint | `MIDPOINT` | Midpoint: Direction changes |
| Plot Point 2 | `PLOT_POINT_2` | Plot Point 2: Enter Act 3 |
| Climax | `CLIMAX` | Climax: Final confrontation |
| Falling Action | `FALLING_ACTION` | Falling Action: Conflict resolution |
| Resolution | `RESOLUTION` | Resolution: New equilibrium |

#### Special Events (8 types)

| Event Type | Enum | Description |
|-----------|------|-------------|
| Twist | `TWIST` | Plot Twist: Unexpected reversal |
| Revelation | `REVELATION` | Revelation: Important information revealed |
| Conflict | `CONFLICT` | Conflict: Opposing forces clash |
| Transition | `TRANSITION` | Transition: Scene/time switch |
| Pause | `PAUSE` | Pause: Slow down pacing |
| Flashback | `FLASHBACK` | Flashback: Past scene |
| Flashforward | `FLASHFORWARD` | Flashforward: Future scene |
| Montage | `MONTAGE` | Montage: Quick cuts |
| Time Skip | `TIME_SKIP` | Time Skip: Jump forward |
| Custom | `CUSTOM` | Custom event |

---

### `StoryBeat`

Story beat structure containing event type and corresponding narrative context:

```cpp
struct StoryBeat {
    DramaticEventType type;    // Event type
    NarrativeContext context;  // Narrative context

    // Constructor
    StoryBeat(DramaticEventType type, const NarrativeContext& context);

    // Convert to string
    std::string to_string() const;
};
```

---

### `NarrativePresets`

Preset narrative context factory, providing standard configurations for all event types.

#### Methods

```cpp
// Get preset context for specific event type
static NarrativeContext get_preset(DramaticEventType type);

// Variable-intensity presets
static NarrativeContext climax(double intensity = 1.0);
static NarrativeContext twist(double magnitude = 0.8);
static NarrativeContext conflict(double intensity = 0.7);

// Flashback and flashforward
static NarrativeContext flashback(double intensity = 0.5, double distance = 0.5);
static NarrativeContext flashforward(double intensity = 0.5, double distance = 0.5);

// Time skip
static NarrativeContext time_skip(double distance = 0.3);

// Fully custom
static NarrativeContext custom(
    double surprisal,
    double logic_deviation,
    double emotional_intensity = 0.0,
    double dramatic_tension = 0.0,
    double time_distance = 0.0
);
```

#### Preset Parameter Table

| Event Type | S | L | E | T | D | Description |
|-----------|---|---|---|---|---|-------------|
| Exposition | 0.1 | 0.0 | 0.2 | 0.1 | 1.0 | Calm introduction |
| Inciting Incident | 0.7 | 0.3 | 0.5 | 0.4 | 0.9 | Inciting incident |
| Rising Action | 0.4 | 0.2 | 0.4 | 0.5 | 0.7 | Rising action |
| Plot Point 1 | 0.6 | 0.5 | 0.6 | 0.7 | 0.6 | First turning point |
| Midpoint | 0.5 | 0.3 | 0.7 | 0.6 | 0.5 | Midpoint |
| Plot Point 2 | 0.7 | 0.6 | 0.8 | 0.8 | 0.4 | Second turning point |
| Climax | 1.0 | 0.8 | 1.0 | 1.0 | 0.0 | Climax |
| Falling Action | 0.3 | 0.2 | 0.4 | 0.3 | 0.2 | Falling action |
| Resolution | 0.1 | 0.0 | 0.2 | 0.1 | 0.0 | Resolution |
| Twist | 0.9 | 0.9 | 0.8 | 0.9 | 0.5 | Plot twist |
| Revelation | 0.8 | 0.4 | 0.7 | 0.6 | 0.5 | Revelation |
| Conflict | 0.6 | 0.3 | 0.7 | 0.8 | 0.5 | Conflict |
| Transition | 0.3 | 0.2 | 0.3 | 0.3 | 0.5 | Transition |
| Pause | 0.2 | 0.1 | 0.2 | 0.1 | 0.5 | Pause |
| Flashback | 0.4 | 0.3 | 0.5 | 0.4 | 0.5 | Flashback |
| Flashforward | 0.5 | 0.4 | 0.6 | 0.5 | 0.5 | Flashforward |
| Montage | 0.4 | 0.2 | 0.5 | 0.5 | 0.5 | Montage |
| Time Skip | 0.6 | 0.3 | 0.4 | 0.3 | 0.5 | Time skip |
| Custom | Variable | Variable | Variable | Variable | Variable | Custom |

---

### `NarrativeStructure`

Narrative structure type enumeration:

```cpp
enum class NarrativeStructure {
    THREE_ACT,         // Three-Act Structure
    HEROS_JOURNEY,     // Hero's Journey
    SAVE_THE_CAT,      // Save the Cat
    CUSTOM             // Custom Structure
};
```

---

### `NarrativeTemplate`

Narrative template providing standard story structure beat sequences.

#### Methods

```cpp
// Get beat sequence for specific structure
static std::vector<StoryBeat> get_template(NarrativeStructure type);

// Three-Act Structure (9 beats)
static std::vector<StoryBeat> three_act_structure();

// Hero's Journey (12 beats)
static std::vector<StoryBeat> heros_journey();

// Save the Cat (15 beats)
static std::vector<StoryBeat> save_the_cat();
```

#### Three-Act Structure Beats

```cpp
{
    {EXPOSITION, {0.1, 0.0, 0.2, 0.1, 1.0}},       // Act 1: Setup
    {INCITING_INCIDENT, {0.7, 0.3, 0.5, 0.4, 0.9}},
    {RISING_ACTION, {0.4, 0.2, 0.4, 0.5, 0.7}},
    {PLOT_POINT_1, {0.6, 0.5, 0.6, 0.7, 0.6}},    // Act 2: Confrontation
    {MIDPOINT, {0.5, 0.3, 0.7, 0.6, 0.5}},
    {RISING_ACTION, {0.4, 0.2, 0.5, 0.7, 0.4}},
    {PLOT_POINT_2, {0.7, 0.6, 0.8, 0.8, 0.4}},    // Act 3: Resolution
    {CLIMAX, {1.0, 0.8, 1.0, 1.0, 0.0}},
    {FALLING_ACTION, {0.3, 0.2, 0.4, 0.3, 0.2}},
    {RESOLUTION, {0.1, 0.0, 0.2, 0.1, 0.0}}
}
```

---

### `NarrativeAnalyzer`

Dynamic narrative analyzer, analyzing current state based on historical and future beats.

#### Constructor

```cpp
NarrativeAnalyzer(const std::vector<StoryBeat>& beats);
```

#### Methods

```cpp
// Analyze narrative state at specific position
NarrativeContext analyze_at(size_t position) const;

// Calculate narrative tension
double calculate_tension() const;

// Detect if approaching climax
bool is_approaching_climax(size_t position, double threshold = 3) const;

// Predict next event type
DramaticEventType predict_next_type(size_t position) const;

// Calculate rhythm (event density)
double calculate_rhythm(size_t position, double window = 5) const;
```

---

## Usage Examples

### Basic Usage

```cpp
#include "cebu/narrative_context.h"

using namespace cebu;

// Create narrative context
NarrativeContext ctx(0.8, 0.7, 0.9, 0.9, 0.0);

// Get driving force and volatility
double driving_force = ctx.get_driving_force();      // κ·S + η·L
double volatility = ctx.get_volatility();            // σ₀ · (1 + E) · (1 + T)

std::cout << "Driving Force: " << driving_force << "\n";
std::cout << "Volatility: " << volatility << "\n";
std::cout << "Context: " << ctx.to_string() << "\n";
```

### Using Presets

```cpp
// Use preset contexts
auto climax_ctx = NarrativePresets::climax(1.0);
auto twist_ctx = NarrativePresets::twist(0.8);
auto conflict_ctx = NarrativePresets::conflict(0.7);

// Custom context
auto custom_ctx = NarrativePresets::custom(
    0.9, 0.8, 0.7, 0.6, 0.3
);
```

### Context Fusion

```cpp
NarrativeContext ctx1(0.8, 0.7, 0.9, 0.9, 0.0);
NarrativeContext ctx2(0.6, 0.5, 0.7, 0.8, 0.2);

// Fuse two contexts (weight 0.5)
auto fused = ctx1.fuse(ctx2, 0.5);
```

### Creating Story Beats

```cpp
// Create story beat
StoryBeat beat(DramaticEventType::CLIMAX,
               NarrativePresets::climax(1.0));

std::cout << beat.to_string() << "\n";
// Output: Climax: S=1.0, L=0.8, E=1.0, T=1.0, D=0.0
```

### Using Narrative Templates

```cpp
// Get Three-Act Structure
auto beats = NarrativeTemplate::three_act_structure();

// Iterate through beats
for (size_t i = 0; i < beats.size(); ++i) {
    std::cout << "Beat " << i << ": " << beats[i].to_string() << "\n";
}

// Get Hero's Journey
auto hero_journey = NarrativeTemplate::heros_journey();
```

### Narrative Analysis

```cpp
// Create analyzer
NarrativeAnalyzer analyzer(beats);

// Analyze state at specific position
auto ctx = analyzer.analyze_at(5);
std::cout << "Analysis: " << ctx.to_string() << "\n";

// Check if approaching climax
if (analyzer.is_approaching_climax(5)) {
    std::cout << "Approaching climax!\n";
}

// Predict next event type
auto next_type = analyzer.predict_next_type(5);
std::cout << "Next event: " << static_cast<int>(next_type) << "\n";

// Calculate rhythm
double rhythm = analyzer.calculate_rhythm(5);
std::cout << "Rhythm: " << rhythm << "\n";
```

### Integration with Absurdity System

```cpp
#include "cebu/absurdity_evolution.h"

// Create narrative context
NarrativeContext ctx(0.8, 0.7, 0.9, 0.9, 0.0);

// Configure absurdity evolution parameters
SDEEvolutionParams params;
params.volatility = ctx.get_volatility();                    // σ
params.coupling_strength = ctx.get_driving_force();          // κ·S + η·L
params.diffusion_strength = 0.1;
params.time_step = 0.01;

// Create evolver
SDEEvolution evolution;

// Evolve absurdity
FuzzyInterval current(0.5, 0.1);
auto evolved = evolution.step(current, params);
```

### Integration with Topology Morph

```cpp
#include "cebu/topology_morph.h"

// Create topology morph system
TopologyMorphSystem morph_system;

// Create story beat
StoryBeat beat(DramaticEventType::CLIMAX,
               NarrativePresets::climax(1.0));

// Trigger morph based on dramatic tension
if (beat.context.dramatic_tension() > 0.8) {
    morph_system.apply(
        ChaosMorphRules(),
        complex,
        beat.context.get_driving_force()
    );
}
```

---

## Complete Demo

### Demo 1: Absurdity & Narrative Evolution

```cpp
#include "cebu/narrative_context.h"
#include "cebu/absurdity_evolution.h"

using namespace cebu;

int main() {
    // Create Three-Act Structure
    auto beats = NarrativeTemplate::three_act_structure();

    // Initialize absurdity
    FuzzyInterval absurdity(0.5, 0.1);
    SDEEvolution evolution;

    std::cout << "=== Absurdity & Narrative Evolution ===\n\n";

    // Iterate through story beats
    for (size_t i = 0; i < beats.size(); ++i) {
        const auto& beat = beats[i];

        std::cout << "Beat " << i << ": " << beat.to_string() << "\n";

        // Configure evolution parameters
        SDEEvolutionParams params;
        params.volatility = beat.context.get_volatility();
        params.coupling_strength = beat.context.get_driving_force();
        params.diffusion_strength = 0.1;
        params.time_step = 0.01;

        // Evolve absurdity
        absurdity = evolution.step(absurdity, params);

        std::cout << "  → Absurdity: " << absurdity.to_string() << "\n";
        std::cout << "  → Volatility: " << params.volatility << "\n";
        std::cout << "  → Driving Force: " << params.coupling_strength << "\n\n";
    }

    return 0;
}
```

---

## Testing

Run test suite:

```bash
./build/test_phase9c_narrative
```

Test coverage:
- Narrative Context basics
- Driving force and volatility calculation
- Context fusion
- Narrative presets
- Narrative structure templates
- Narrative analyzer

---

## Performance Characteristics

| Operation | Time Complexity | Space Complexity |
|-----------|-----------------|------------------|
| Create context | O(1) | O(1) |
| Get driving force/volatility | O(1) | O(1) |
| Context fusion | O(1) | O(1) |
| Get template | O(n) | O(n) |
| Narrative analysis | O(n) | O(n) |

---

## Related Modules

- **Phase 9a**: Absurdity System (`absurdity.h`)
- **Phase 9b**: Topology Morph (`topology_morph.h`)
- **Narrative System**: `story_event.h`, `simplicial_complex_narrative.h`, `timeline.h`

---

## References

- [Absurdity Requirements](../prepare/荒谬度.md)
- [Phase 9a Completion](.codebuddy/phase9a_summary.md)
- [Phase 9b Completion](.codebuddy/phase9b_summary.md)
