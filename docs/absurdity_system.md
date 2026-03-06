# Absurdity System

The absurdity system is the core innovation of Cebu - a semantic layer that drives topological changes through narrative and emotional metrics. This document covers Phase 9a and Phase 9b implementations.

## Table of Contents

1. [Overview](#overview)
2. [Fuzzy Interval Numbers](#fuzzy-interval-numbers)
3. [Stochastic Evolution](#stochastic-evolution)
4. [Multi-Source Fusion](#multi-source-fusion)
5. [Absurdity Field](#absurdity-field)
6. [Dynamic Topology Morph](#dynamic-topology-morph)
7. [Usage Examples](#usage-examples)
8. [API Reference](#api-reference)

---

## Overview

The absurdity system transforms narrative and emotional metrics into topological changes:

```
Narrative Context → Absurdity Values → Topology Morphs → Structural Change
```

**Key Features**:
- **Uncertainty-Aware**: Fuzzy interval numbers express measurement uncertainty
- **Stochastic Evolution**: Random differential equations model dynamic behavior
- **Multi-Source Fusion**: Combine absurdity from multiple sensors/observers
- **Topology-Driven**: Absurdity values trigger structural deformations

---

## Fuzzy Interval Numbers

### Concept

A fuzzy interval number represents uncertainty as:

\[
\mathcal{A} = [\alpha^-, \alpha^+] \times \mu
\]

Where:
- \([\alpha^-, \alpha^+]\): Plausible range of absurdity values
- \(\mu \in [0,1]\): Confidence in our measurement

**Example**: `[0.7, 0.9] × 0.8` means we're 80% confident the absurdity is between 0.7 and 0.9.

### Basic Usage

```cpp
#include "cebu/absurdity.h"

// Create a fuzzy interval
cebu::FuzzyInterval a(0.7, 0.9, 0.8);  // [0.7, 0.9] × 0.8

// Access properties
double lower = a.lower;        // 0.7
double upper = a.upper;        // 0.9
double confidence = a.confidence; // 0.8
double midpoint = a.midpoint();  // 0.8
double width = a.width();       // 0.2

// Check properties
bool is_crisp = a.is_crisp();      // false (width > 0)
bool is_certain = a.is_certain();   // false (confidence < 0.9)

// String representation
std::cout << a.to_string() << std::endl;  // "[0.700, 0.900] × 0.800"
std::cout << a.to_latex() << std::endl;   // "[0.700, 0.900] \\times 0.800"
```

### Arithmetic Operations

```cpp
cebu::FuzzyInterval a(0.3, 0.5, 0.8);
cebu::FuzzyInterval b(0.4, 0.6, 0.9);

// Addition
cebu::FuzzyInterval sum = a + b;  // [0.7, 1.1] × 0.85

// Subtraction
cebu::FuzzyInterval diff = a - b;  // [-0.3, 0.1] × 0.85

// Multiplication
cebu::FuzzyInterval product = a * b;  // [0.12, 0.3] × 0.85

// Scalar multiplication
cebu::FuzzyInterval scaled = a * 2.0;  // [0.6, 1.0] × 0.8

// Division (throws if divisor contains zero)
cebu::FuzzyInterval ratio = a / b;
```

### Uncertainty Operations

```cpp
cebu::FuzzyInterval a(0.3, 0.6, 0.8);
cebu::FuzzyInterval b(0.5, 0.7, 0.9);

// Intersection (common range)
cebu::FuzzyInterval inter = a.intersect(b);  // [0.5, 0.6] × ...

// Union (combined range)
cebu::FuzzyInterval uni = a.union_with(b);  // [0.3, 0.7] × ...

// Overlap degree (0.0 to 1.0)
double overlap = a.overlap_degree(b);  // 0.333...
```

### Comparison with Uncertainty

```cpp
cebu::FuzzyInterval a(0.3, 0.5, 0.8);
cebu::FuzzyInterval b(0.6, 0.8, 0.9);

// Uncertainty-aware comparison
bool less = a < b;    // true (with uncertainty)
bool greater = a > b; // false
bool equal = a == b;   // false

// Check if definitely less
bool def_less = cebu::UncertaintyComparison::definitely_less(a, b);

// Probability that a < b
double prob = cebu::UncertaintyComparison::probability_less(a, b);

// Full comparison result
auto result = cebu::UncertaintyComparison::compare(a, b);
// result.less: probability that a < b
// result.equal: probability that a ≈ b
// result.greater: probability that a > b
```

---

## Stochastic Evolution

### Concept

Absurdity evolves according to a stochastic differential equation:

\[
d\mathcal{A} = (\kappa \cdot (S - \mathcal{A}) + \eta \cdot L) dt + \sigma \cdot \mathcal{A} \cdot dW
\]

Where:
- \(S\): Surprisal (narrative context)
- \(L\): Logical deviation (1 - rationality)
- \(\kappa, \eta\): Coupling constants
- \(\sigma\): Noise intensity
- \(dW\): Wiener process (Gaussian noise)

### Basic Evolution

```cpp
#include "cebu/absurdity.h"

// Define evolution parameters
cebu::EvolutionParams params;
params.decay_rate = 0.95;          // Exponential decay per step
params.diffusion_rate = 0.05;       // Spatial diffusion
params.volatility = 0.1;           // Noise magnitude
params.coupling_strength = 0.3;    // Neighbor influence
params.threshold = 0.5;            // Activation threshold

// Create evolution engine
cebu::StochasticEvolution evolution(params);

// Evolve a single value
cebu::FuzzyInterval current(0.5, 0.6, 0.8);
cebu::FuzzyInterval next = evolution.step(current, 0.1);  // dt = 0.1

// Evolve over multiple steps
auto trajectory = evolution.evolve(current, 10);
// trajectory[0]: initial
// trajectory[1]: after 1 step
// ...
// trajectory[10]: after 10 steps
```

### Neighborhood Evolution

```cpp
// Evolve values with neighbor coupling
std::vector<cebu::FuzzyInterval> values = {
    cebu::FuzzyInterval(0.3, 0.5, 0.8),
    cebu::FuzzyInterval(0.4, 0.6, 0.9),
    cebu::FuzzyInterval(0.5, 0.7, 0.85)
};

// Define adjacency (neighbors for each value)
std::vector<std::vector<size_t>> adjacency = {
    {1},        // Value 0 neighbors: value 1
    {0, 2},     // Value 1 neighbors: values 0, 2
    {1}         // Value 2 neighbors: value 1
};

// Evolve with neighbor coupling
auto next_values = evolution.evolve_neighborhood(values, adjacency);
```

### Noise Generators

```cpp
// Gaussian noise (default)
cebu::NoiseGenerator noise(
    cebu::NoiseGenerator::Type::Gaussian,
    0.0,  // mean
    0.1   // std deviation
);

// Uniform noise
cebu::NoiseGenerator uniform_noise(
    cebu::NoiseGenerator::Type::Uniform,
    0.0,  // min
    1.0   // max
);

// Poisson noise
cebu::NoiseGenerator poisson_noise(
    cebu::NoiseGenerator::Type::Poisson,
    5.0   // lambda
);

// Levy flight (heavy-tailed)
cebu::NoiseGenerator levy_noise(
    cebu::NoiseGenerator::Type::Levy,
    0.0,  // location
    0.1,  // scale
    1.5   // exponent
);

// Generate noise
double n = noise();  // Sample from distribution

// Seed for reproducibility
noise.seed(12345);
```

---

## Multi-Source Fusion

### Concept

When multiple observers/sensors provide absurdity estimates, fuse them using various strategies:

\[
\mathcal{A}_{\text{fused}} = \bigoplus_{i=1}^n w_i \mathcal{A}_i
\]

### Fusion Strategies

```cpp
#include "cebu/absurdity.h"

std::vector<cebu::FuzzyInterval> sources = {
    cebu::FuzzyInterval(0.3, 0.5, 0.8),
    cebu::FuzzyInterval(0.4, 0.6, 0.9),
    cebu::FuzzyInterval(0.5, 0.7, 0.85)
};

std::vector<double> weights = {0.3, 0.5, 0.2};  // Must sum to 1.0

// Weighted average (default)
auto fused_avg = cebu::FusionStrategy::fuse(
    sources,
    weights,
    cebu::FusionStrategy::Method::WeightedAverage
);

// Dempster-Shafer (evidence theory)
auto fused_ds = cebu::FusionStrategy::fuse(
    sources,
    weights,
    cebu::FusionStrategy::Method::DempsterShafer
);

// Bayesian (probabilistic)
auto fused_bayes = cebu::FusionStrategy::fuse(
    sources,
    weights,
    cebu::FusionStrategy::Method::Bayesian
);

// Fuzzy logic (T-conorm)
auto fused_fuzzy = cebu::FusionStrategy::fuse(
    sources,
    weights,
    cebu::FusionStrategy::Method::FuzzyLogic
);

// Possibility theory
auto fused_possible = cebu::FusionStrategy::fuse(
    sources,
    weights,
    cebu::FusionStrategy::Method::Possibility
);

// Adaptive (automatically chooses best method)
auto fused_adaptive = cebu::FusionStrategy::fuse(
    sources,
    weights,
    cebu::FusionStrategy::Method::Adaptive
);
```

### Fusion Method Selection

```cpp
// Analyze sources to recommend method
auto recommendation = cebu::FusionStrategy::recommend_method(sources);

std::cout << "Recommended: " << recommendation.method << std::endl;
std::cout << "Confidence: " << recommendation.confidence << std::endl;
```

---

## Absurdity Field

### Concept

The absurdity field manages absurdity values across all simplices in a complex:

\[
\mathcal{A}: \text{SimplexID} \rightarrow [\alpha^-, \alpha^+] \times \mu
\]

### Basic Operations

```cpp
#include "cebu/absurdity.h"

cebu::SimplicialComplex complex;
// ... build complex ...

cebu::AbsurdityField field;

// Set absurdity for a simplex
cebu::SimplexID simplex_id = /* ... */;
field.set(simplex_id, cebu::FuzzyInterval(0.7, 0.9, 0.85));

// Get absurdity
auto absurdity = field.get(simplex_id);

// Check if simplex has absurdity
bool has_absurdity = field.has(simplex_id);

// Remove absurdity
field.remove(simplex_id);

// Clear all values
field.clear();
```

### Field Evolution

```cpp
// Update entire field
cebu::EvolutionParams params;
cebu::StochasticEvolution evolution(params);

// Evolve all values
field.evolve(evolution, 0.1);  // dt = 0.1

// Evolve specific region
std::vector<cebu::SimplexID> region = {id1, id2, id3};
field.evolve_region(region, evolution, 0.1);
```

### Spatial Diffusion

```cpp
// Diffuse absurdity across neighbors (smoothing)
double diffusion_rate = 0.1;
int iterations = 5;

field.diffuse(diffusion_rate, iterations);

// This models "absurdity infection" across the complex
```

### Query Operations

```cpp
// Find simplices above threshold
auto high_absurdity = field.find_above_threshold(
    cebu::FuzzyInterval(0.7, 0.8, 0.9)
);

// Find simplices below threshold
auto low_absurdity = field.find_below_threshold(
    cebu::FuzzyInterval(0.2, 0.3, 0.9)
);

// Find in range
auto in_range = field.find_in_range(
    cebu::FuzzyInterval(0.4, 0.6, 0.9),
    cebu::FuzzyInterval(0.3, 0.7, 0.9)
);

// Get statistics
auto stats = field.get_statistics();
std::cout << "Mean: " << stats.mean << std::endl;
std::cout << "Std dev: " << stats.std_dev << std::endl;
std::cout << "Min: " << stats.min.to_string() << std::endl;
std::cout << "Max: " << stats.max.to_string() << std::endl;
```

### Absurdity Labels

```cpp
// Convert absurdity to human-readable label
cebu::AbsurdityLabel label = cebu::absurdity_to_label(
    cebu::FuzzyInterval(0.85, 0.95, 0.98)
);

std::cout << label.name << std::endl;           // "EXTREME"
std::cout << label.description << std::endl;    // Description
std::cout << label.color << std::endl;          // RGB color

// Get all available labels
auto all_labels = cebu::AbsurdityLabel::all_labels();
for (const auto& l : all_labels) {
    std::cout << l.name << ": " << l.range.to_string() << std::endl;
}
```

---

## Dynamic Topology Morph

### Concept

When absurdity exceeds thresholds, trigger topological deformations:

```
High Absurdity → Split/Morph → More Complexity
Low Absurdity → Merge/Contract → Less Complexity
```

### Morph Operations

**10 Morph Types**:

1. **SPLIT**: Break simplex into pieces
2. **MERGE**: Combine adjacent simplices
3. **DELETE**: Remove simplex
4. **CREATE**: Add new simplex
5. **EXPAND**: Increase dimension (edge → face)
6. **CONTRACT**: Decrease dimension (face → edge)
7. **GLUE**: Identify vertices
8. **REFINE**: Add interior vertex
9. **COARSEN**: Remove interior vertex
10. **WRAP**: Create non-orientable topology

### Basic Morph Usage

```cpp
#include "cebu/topology_morph.h"

cebu::SimplicialComplex complex;
cebu::AbsurdityField field;

// ... build complex and set absurdity ...

// Create morph rule
cebu::MorphRule rule(
    "high_absurdity_split",
    cebu::MorphType::SPLIT,
    cebu::TriggerCondition::ABSURDITY_THRESHOLD,
    cebu::FuzzyInterval(0.7, 0.8, 0.9)
);
rule.probability = 0.8;  // 80% chance
rule.priority = 10;     // Higher = checked first

// Create morph engine
cebu::TopologyMorph morph;
morph.add_rule(rule);

// Apply morph
cebu::MorphContext context(complex, field, 0.0, 0.1);
auto result = morph.apply_morph(rule, simplex_id, context);

if (result.success) {
    std::cout << "Success: " << result.message << std::endl;
    std::cout << "Created: " << result.created.size() << " simplices" << std::endl;
    std::cout << "Removed: " << result.removed.size() << " simplices" << std::endl;
}
```

### Morph Presets

```cpp
// Default rules (balanced)
auto default_rules = cebu::MorphPresets::default_rules();

// Conservative rules (minimal changes)
auto conservative = cebu::MorphPresets::conservative_rules();

// Aggressive rules (lots of deformation)
auto aggressive = cebu::MorphPresets::aggressive_rules();

// Chaos mode (extreme deformation)
auto chaos = cebu::MorphPresets::chaos_rules();

// Artistic rules (aesthetic, controlled)
auto artistic = cebu::MorphPresets::artistic_rules();

// Scientific rules (structure-preserving)
auto scientific = cebu::MorphPresets::scientific_rules();

// Load preset
cebu::TopologyMorph morph;
for (const auto& rule : conservative) {
    morph.add_rule(rule);
}
```

### Batch Rule Application

```cpp
// Apply all matching rules
auto results = morph.apply_rules(context, max_operations);

// Iterate results
for (const auto& result : results) {
    if (result.success) {
        std::cout << "Applied: " << result.message << std::endl;
    }
}

// Check statistics
const auto& stats = morph.get_stats();
std::cout << "Total operations: " << stats.total_operations << std::endl;
std::cout << "Successful: " << stats.successful_operations << std::endl;
std::cout << "Splits: " << stats.operation_counts[cebu::MorphType::SPLIT] << std::endl;
```

### Deferred Morphs

```cpp
// Queue morphs for delayed execution
cebu::MorphQueue queue;

queue.enqueue(cebu::QueuedMorph(rule, simplex_id, 0.5));  // 0.5s delay
queue.enqueue(cebu::QueuedMorph(rule2, simplex_id2, 1.0)); // 1.0s delay

// Execute ready morphs
auto results = queue.execute_ready(current_time, context);

// Cancel morphs
queue.cancel_for_simplex(simplex_id);
```

### Complete Evolution Example

```cpp
// Setup
cebu::SimplicialComplex complex;
cebu::AbsurdityField field;
cebu::TopologyMorph morph;
cebu::StochasticEvolution evolution;

// Load rules
auto rules = cebu::MorphPresets::default_rules();
for (const auto& rule : rules) {
    morph.add_rule(rule);
}

// Evolve over time
for (double t = 0.0; t < 10.0; t += 0.1) {
    // Evolve absurdity
    for (auto [id, simplex] : complex.get_simplices()) {
        auto current = field.get(id);
        auto next = evolution.step(current, 0.1);
        field.set(id, next);
    }
    
    // Apply morph rules
    cebu::MorphContext context(complex, field, t, 0.1);
    morph.apply_rules(context);
    
    std::cout << "Time " << t << ": " 
              << complex.get_simplices().size() << " simplices" << std::endl;
}
```

---

## Usage Examples

### Example 1: Interactive Absurdity

```cpp
// User interaction increases absurdity in focused region
void on_user_focus(SimplexID focused_simplex) {
    // Increase absurdity
    auto current = field.get(focused_simplex);
    auto increased = current * 1.5;  // 50% increase
    field.set(focused_simplex, increased);
    
    // Trigger morph check
    MorphContext context(complex, field, time, dt);
    morph.apply_rules(context);
}
```

### Example 2: Narrative-Driven Morph

```cpp
// Story events drive topology changes
void on_plot_twist() {
    // Boost absurdity globally
    for (auto [id, simplex] : complex.get_simplices()) {
        auto current = field.get(id);
        auto boosted = cebu::FuzzyInterval(
            std::min(0.95, current.lower + 0.3),
            std::min(1.0, current.upper + 0.3),
            current.confidence * 0.9
        );
        field.set(id, boosted);
    }
    
    // Morph triggers naturally
}
```

### Example 3: Sensor Fusion

```cpp
// Combine multiple sensor inputs
void update_from_sensors(
    const std::vector<cebu::FuzzyInterval>& sensor_readings,
    const std::vector<double>& sensor_reliabilities
) {
    // Fuse readings
    auto fused = cebu::FusionStrategy::fuse(
        sensor_readings,
        sensor_reliabilities,
        cebu::FusionStrategy::Method::Adaptive
    );
    
    // Update field
    field.set(target_simplex, fused);
}
```

---

## API Reference

### Core Classes

#### FuzzyInterval

```cpp
class FuzzyInterval {
public:
    double lower;           // Lower bound
    double upper;           // Upper bound
    double confidence;      // Confidence [0,1]
    
    FuzzyInterval(double l, double u, double c);
    
    double midpoint() const;
    double width() const;
    bool is_crisp() const;
    bool is_certain() const;
    
    bool operator<(const FuzzyInterval&) const;
    bool operator<=(const FuzzyInterval&) const;
    bool operator>(const FuzzyInterval&) const;
    bool operator>=(const FuzzyInterval&) const;
    bool operator==(const FuzzyInterval&) const;
    bool operator!=(const FuzzyInterval&) const;
    
    FuzzyInterval operator+(const FuzzyInterval&) const;
    FuzzyInterval operator-(const FuzzyInterval&) const;
    FuzzyInterval operator*(const FuzzyInterval&) const;
    FuzzyInterval operator/(const FuzzyInterval&) const;
    FuzzyInterval operator*(double) const;
    
    FuzzyInterval intersect(const FuzzyInterval&) const;
    FuzzyInterval union_with(const FuzzyInterval&) const;
    double overlap_degree(const FuzzyInterval&) const;
    
    std::string to_string() const;
    std::string to_latex() const;
};
```

#### AbsurdityField

```cpp
class AbsurdityField {
public:
    void set(SimplexID id, const FuzzyInterval& absurdity);
    FuzzyInterval get(SimplexID id) const;
    bool has(SimplexID id) const;
    void remove(SimplexID id);
    void clear();
    
    void evolve(const StochasticEvolution&, double dt);
    void evolve_region(const std::vector<SimplexID>&,
                      const StochasticEvolution&, double dt);
    
    void diffuse(double rate, int iterations);
    
    std::vector<SimplexID> find_above_threshold(
        const FuzzyInterval& threshold) const;
    std::vector<SimplexID> find_below_threshold(
        const FuzzyInterval& threshold) const;
    
    Statistics get_statistics() const;
};
```

#### TopologyMorph

```cpp
class TopologyMorph {
public:
    void add_rule(const MorphRule&);
    bool remove_rule(const std::string& name);
    const std::vector<MorphRule>& get_rules() const;
    
    std::vector<MorphResult> apply_rules(
        MorphContext& context,
        size_t max_operations = 0);
    
    const MorphRule* find_matching_rule(
        SimplexID simplex_id,
        const MorphContext& context) const;
    
    MorphResult apply_morph(
        const MorphRule& rule,
        SimplexID simplex_id,
        MorphContext& context);
    
    const Stats& get_stats() const;
    void reset_stats();
    
    void set_enabled(bool enabled);
    bool is_enabled() const;
    
    // Static morph operations
    static MorphResult split_simplex(
        SimplicialComplex& complex,
        SimplexID simplex_id,
        const AbsurdityField& field,
        size_t pieces = 2);
    
    static MorphResult merge_simplices(
        SimplicialComplex& complex,
        SimplexID simplex_id,
        const AbsurdityField& field);
    
    // ... other morph operations
};
```

### Utility Classes

#### StochasticEvolution

```cpp
class StochasticEvolution {
public:
    explicit StochasticEvolution(const EvolutionParams& params);
    
    FuzzyInterval step(const FuzzyInterval& current, double dt);
    std::vector<FuzzyInterval> evolve(const FuzzyInterval& initial, int steps);
    
    std::vector<FuzzyInterval> evolve_neighborhood(
        const std::vector<FuzzyInterval>& values,
        const std::vector<std::vector<size_t>>& adjacency);
};
```

#### FusionStrategy

```cpp
class FusionStrategy {
public:
    enum class Method {
        WeightedAverage,
        DempsterShafer,
        Bayesian,
        FuzzyLogic,
        Possibility,
        Adaptive
    };
    
    static FuzzyInterval fuse(
        const std::vector<FuzzyInterval>& sources,
        const std::vector<double>& weights,
        Method method = Method::WeightedAverage);
    
    static MethodRecommendation recommend_method(
        const std::vector<FuzzyInterval>& sources);
};
```

---

## Performance Considerations

### Complexity

| Operation | Time Complexity | Space Complexity |
|-----------|----------------|------------------|
| FuzzyInterval operations | O(1) | O(1) |
| Evolution step | O(1) | O(1) |
| Field evolution (all) | O(n) | O(n) |
| Diffusion | O(n·k) | O(n) |
| Morph rule matching | O(n·r) | O(1) |
| Single morph | O(k) | O(k) |

Where:
- n = number of simplices
- r = number of rules
- k = morph complexity
- k = diffusion iterations

### Optimization Tips

1. **Batch Operations**: Evolve entire field at once
2. **Spatial Indexing**: Use BVH/Octree for regional queries
3. **Rule Limiting**: Limit rules applied per frame
4. **Probability Thresholds**: Use low probability for aggressive rules
5. **Lazy Evaluation**: Only evaluate rules when needed

---

## Best Practices

### 1. Absurdity Range

- Keep absurdity in `[0, 1]` range
- Use confidence to express uncertainty
- Clamp values if they drift out of range

### 2. Evolution Parameters

- **Decay rate**: 0.9-0.99 for smooth evolution
- **Volatility**: 0.05-0.2 for controlled randomness
- **Coupling strength**: 0.2-0.5 for neighbor influence

### 3. Morph Rules

- Use **priority** to order rule checking
- Set **probability** to control frequency
- Use **max_applications** to prevent excessive morphs
- Test rules in isolation first

### 4. Performance

- Limit field size for real-time applications
- Use spatial indexing for large complexes
- Batch morph operations
- Profile evolution and morph costs

---

## Troubleshooting

### Issue: Complex grows too large

**Solution**: Add conservative rules or increase thresholds
```cpp
MorphRule conservative("conservative", MorphType::SPLIT, 
    TriggerCondition::ABSURDITY_THRESHOLD,
    FuzzyInterval(0.9, 0.95, 0.98));  // Higher threshold
```

### Issue: Morphs too frequent

**Solution**: Reduce probability or add max applications
```cpp
rule.probability = 0.3;  // 30% chance
rule.max_applications = 10;  // Limit to 10 applications
```

### Issue: Absurdity becomes unstable

**Solution**: Reduce volatility or increase decay rate
```cpp
params.volatility = 0.05;  // Lower noise
params.decay_rate = 0.98;   // Faster decay
```

---

## See Also

- [API Reference](api.md) - Complete API documentation
- [Topology Operations](topology_operations.md) - Topological operations
- [Performance Guide](performance.md) - Performance optimization
