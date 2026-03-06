#include "cebu/absurdity.h"
#include <iostream>
#include <vector>
#include <iomanip>

using namespace cebu;

// =============================================================================
// Test 1: Basic Fuzzy Interval Operations
// =============================================================================
void test_fuzzy_interval_basics() {
    std::cout << "=== Test 1: Fuzzy Interval Basics ===" << std::endl;
    
    // Create intervals
    FuzzyInterval a(0.3, 0.5, 0.9);
    FuzzyInterval b(0.4, 0.7, 0.8);
    
    std::cout << "a = " << a.to_string() << std::endl;
    std::cout << "b = " << b.to_string() << std::endl;
    
    // Arithmetic
    FuzzyInterval sum = a + b;
    FuzzyInterval diff = a - b;
    FuzzyInterval prod = a * b;
    
    std::cout << "a + b = " << sum.to_string() << std::endl;
    std::cout << "a - b = " << diff.to_string() << std::endl;
    std::cout << "a * b = " << prod.to_string() << std::endl;
    
    // Comparison
    std::cout << "a < b: " << (a < b ? "true" : "false") << std::endl;
    std::cout << "a > b: " << (a > b ? "true" : "false") << std::endl;
    std::cout << "a == b: " << (a == b ? "true" : "false") << std::endl;
    
    // Set operations
    FuzzyInterval inter = a.intersect(b);
    FuzzyInterval uni = a.union_with(b);
    
    std::cout << "intersection = " << inter.to_string() << std::endl;
    std::cout << "union = " << uni.to_string() << std::endl;
    std::cout << "overlap degree = " << a.overlap_degree(b) << std::endl;
    
    std::cout << std::endl;
}

// =============================================================================
// Test 2: Stochastic Evolution
// =============================================================================
void test_stochastic_evolution() {
    std::cout << "=== Test 2: Stochastic Evolution ===" << std::endl;
    
    EvolutionParams params;
    params.decay_rate = 0.95;
    params.volatility = 0.1;
    params.coupling_strength = 0.2;
    params.diffusion_rate = 0.05;
    
    StochasticEvolution evolution(params);
    
    FuzzyInterval initial(0.5, 0.5, 1.0);
    std::cout << "Initial: " << initial.to_string() << std::endl;
    
    // Evolve for 10 steps
    auto trajectory = evolution.evolve(initial, 10);
    
    std::cout << "Evolution trajectory:" << std::endl;
    for (size_t i = 0; i < trajectory.size(); ++i) {
        if (i % 2 == 0 || i == trajectory.size() - 1) {
            std::cout << "  Step " << std::setw(2) << i << ": " 
                      << trajectory[i].to_string() << std::endl;
        }
    }
    
    std::cout << std::endl;
}

// =============================================================================
// Test 3: Neighborhood Evolution
// =============================================================================
void test_neighborhood_evolution() {
    std::cout << "=== Test 3: Neighborhood Evolution ===" << std::endl;
    
    EvolutionParams params;
    params.decay_rate = 0.98;
    params.volatility = 0.05;
    params.coupling_strength = 0.3;
    params.diffusion_rate = 0.1;
    
    StochasticEvolution evolution(params);
    
    // Create a simple 1D chain
    std::vector<FuzzyInterval> values(5);
    values[0] = FuzzyInterval(0.8, 0.8, 1.0);  // High at left
    values[1] = FuzzyInterval(0.6, 0.6, 1.0);
    values[2] = FuzzyInterval(0.4, 0.4, 1.0);  // Middle
    values[3] = FuzzyInterval(0.2, 0.2, 1.0);
    values[4] = FuzzyInterval(0.1, 0.1, 1.0);  // Low at right
    
    // Adjacency (1D chain)
    std::vector<std::vector<size_t>> adjacency(5);
    adjacency[0] = {1};
    adjacency[1] = {0, 2};
    adjacency[2] = {1, 3};
    adjacency[3] = {2, 4};
    adjacency[4] = {3};
    
    std::cout << "Initial values:" << std::endl;
    for (size_t i = 0; i < values.size(); ++i) {
        std::cout << "  [" << i << "] = " << values[i].to_string() << std::endl;
    }
    
    // Evolve for 5 steps
    for (int step = 0; step < 5; ++step) {
        values = evolution.evolve_neighborhood(values, adjacency);
        
        std::cout << "After step " << step + 1 << ":" << std::endl;
        for (size_t i = 0; i < values.size(); ++i) {
            std::cout << "  [" << i << "] = " << values[i].to_string() << std::endl;
        }
    }
    
    std::cout << std::endl;
}

// =============================================================================
// Test 4: Multi-Source Fusion
// =============================================================================
void test_fusion() {
    std::cout << "=== Test 4: Multi-Source Fusion ===" << std::endl;
    
    // Create multiple sources
    std::vector<AbsurditySource> sources = {
        {"Source A", FuzzyInterval(0.3, 0.5, 0.9), 0.9, 1.0, "Expert opinion"},
        {"Source B", FuzzyInterval(0.4, 0.6, 0.8), 0.7, 0.8, "Historical data"},
        {"Source C", FuzzyInterval(0.2, 0.4, 0.95), 0.95, 1.2, "Measurement"}
    };
    
    std::cout << "Sources:" << std::endl;
    for (const auto& src : sources) {
        std::cout << "  " << src.name << ": " << src.value.to_string()
                  << " (reliability=" << src.reliability << ", weight=" << src.weight << ")" << std::endl;
    }
    
    // Compute conflict
    double conflict = FusionStrategy::compute_conflict(sources);
    std::cout << "Conflict level: " << conflict << std::endl;
    
    // Select method
    auto method = FusionStrategy::select_method(sources);
    std::cout << "Selected method: ";
    switch (method) {
        case FusionStrategy::Method::WeightedAverage: std::cout << "Weighted Average"; break;
        case FusionStrategy::Method::DempsterShafer: std::cout << "Dempster-Shafer"; break;
        case FusionStrategy::Method::Bayesian: std::cout << "Bayesian"; break;
        case FusionStrategy::Method::FuzzyLogic: std::cout << "Fuzzy Logic"; break;
        case FusionStrategy::Method::PossibilityTheory: std::cout << "Possibility Theory"; break;
        case FusionStrategy::Method::Adaptive: std::cout << "Adaptive"; break;
    }
    std::cout << std::endl;
    
    // Fuse with different methods
    std::cout << "\nFusion results:" << std::endl;
    std::cout << "  Weighted Average: " << FusionStrategy::fuse(sources, FusionStrategy::Method::WeightedAverage).to_string() << std::endl;
    std::cout << "  Dempster-Shafer:   " << FusionStrategy::fuse(sources, FusionStrategy::Method::DempsterShafer).to_string() << std::endl;
    std::cout << "  Bayesian:          " << FusionStrategy::fuse(sources, FusionStrategy::Method::Bayesian).to_string() << std::endl;
    std::cout << "  Fuzzy Logic:       " << FusionStrategy::fuse(sources, FusionStrategy::Method::FuzzyLogic).to_string() << std::endl;
    std::cout << "  Possibility:       " << FusionStrategy::fuse(sources, FusionStrategy::Method::PossibilityTheory).to_string() << std::endl;
    std::cout << "  Adaptive:          " << FusionStrategy::fuse(sources, FusionStrategy::Method::Adaptive).to_string() << std::endl;
    
    std::cout << std::endl;
}

// =============================================================================
// Test 5: Uncertainty Comparison
// =============================================================================
void test_uncertainty_comparison() {
    std::cout << "=== Test 5: Uncertainty Comparison ===" << std::endl;
    
    FuzzyInterval a(0.2, 0.3, 0.9);
    FuzzyInterval b(0.25, 0.35, 0.8);
    FuzzyInterval c(0.4, 0.6, 0.95);
    FuzzyInterval d(0.1, 0.2, 1.0);
    
    std::cout << "Comparing intervals:" << std::endl;
    std::cout << "  a = " << a.to_string() << std::endl;
    std::cout << "  b = " << b.to_string() << std::endl;
    std::cout << "  c = " << c.to_string() << std::endl;
    std::cout << "  d = " << d.to_string() << std::endl;
    
    auto compare = [](const FuzzyInterval& x, const FuzzyInterval& y) {
        auto result = UncertaintyComparison::compare(x, y);
        std::cout << "  " << x.to_string() << " vs " << y.to_string() << ": "
                  << result.explanation << " (confidence=" << result.confidence << ")" << std::endl;
    };
    
    compare(a, b);
    compare(a, c);
    compare(a, d);
    compare(b, c);
    
    std::cout << "\nBinary predicates:" << std::endl;
    std::cout << "  a definitely less than b: " << (UncertaintyComparison::definitely_less(a, b) ? "true" : "false") << std::endl;
    std::cout << "  a possibly less than b: " << (UncertaintyComparison::possibly_less(a, b) ? "true" : "false") << std::endl;
    std::cout << "  a possibly equal to b: " << (UncertaintyComparison::possibly_equal(a, b) ? "true" : "false") << std::endl;
    std::cout << "  P(a < b) = " << UncertaintyComparison::probability_less(a, b) << std::endl;
    
    std::cout << std::endl;
}

// =============================================================================
// Test 6: Absurdity Field
// =============================================================================
void test_absurdity_field() {
    std::cout << "=== Test 6: Absurdity Field ===" << std::endl;
    
    AbsurdityField field;
    
    // Set values
    for (size_t i = 0; i < 10; ++i) {
        double val = static_cast<double>(i) / 10.0;
        field.set(i, FuzzyInterval(val, val, 1.0));
    }
    
    std::cout << "Field values:" << std::endl;
    for (size_t i = 0; i < 10; ++i) {
        std::cout << "  [" << i << "] = " << field.get(i).to_string() << std::endl;
    }
    
    // Statistics
    std::cout << "\nStatistics:" << std::endl;
    std::cout << "  Mean:     " << field.mean().to_string() << std::endl;
    std::cout << "  Variance: " << field.variance().to_string() << std::endl;
    std::cout << "  Min:      " << field.min().to_string() << std::endl;
    std::cout << "  Max:      " << field.max().to_string() << std::endl;
    
    // Query
    auto above = field.find_above_threshold(0.5);
    auto below = field.find_below_threshold(0.5);
    auto extremes = field.find_extremes(3);
    
    std::cout << "\nQueries:" << std::endl;
    std::cout << "  Above 0.5: ";
    for (size_t id : above) std::cout << id << " ";
    std::cout << std::endl;
    std::cout << "  Below 0.5: ";
    for (size_t id : below) std::cout << id << " ";
    std::cout << std::endl;
    std::cout << "  Top 3: ";
    for (size_t id : extremes) std::cout << id << " ";
    std::cout << std::endl;
    
    // Diffusion
    field.diffuse(0.1, 5);
    std::cout << "\nAfter diffusion (rate=0.1, 5 iterations):" << std::endl;
    for (size_t i = 0; i < 10; ++i) {
        std::cout << "  [" << i << "] = " << field.get(i).to_string() << std::endl;
    }
    
    std::cout << std::endl;
}

// =============================================================================
// Test 7: Utility Functions
// =============================================================================
void test_utilities() {
    std::cout << "=== Test 7: Utility Functions ===" << std::endl;
    
    // Create intervals
    auto crisp_val = absurdity_utils::crisp(0.5);
    auto uncertain_val = absurdity_utils::uncertain(0.5, 0.1, 0.8);
    auto random_val = absurdity_utils::random(0.2, 0.8);
    
    std::cout << "Crisp:       " << crisp_val.to_string() << std::endl;
    std::cout << "Uncertain:   " << uncertain_val.to_string() << std::endl;
    std::cout << "Random:      " << random_val.to_string() << std::endl;
    
    // Interpolation
    FuzzyInterval a(0.2, 0.3, 1.0);
    FuzzyInterval b(0.6, 0.7, 0.8);
    auto interp = absurdity_utils::interpolate(a, b, 0.5);
    std::cout << "\nInterpolation:" << std::endl;
    std::cout << "  a = " << a.to_string() << std::endl;
    std::cout << "  b = " << b.to_string() << std::endl;
    std::cout << "  midpoint (t=0.5) = " << interp.to_string() << std::endl;
    
    // Distance and similarity
    double dist = absurdity_utils::distance(a, b);
    double sim = absurdity_utils::similarity(a, b);
    std::cout << "  Distance: " << dist << std::endl;
    std::cout << "  Similarity: " << sim << std::endl;
    
    // Clamp
    auto clamped = absurdity_utils::clamp(FuzzyInterval(-0.2, 1.3, 0.9), 0.0, 1.0);
    std::cout << "  Clamp([-0.2, 1.3], [0, 1]) = " << clamped.to_string() << std::endl;
    
    std::cout << std::endl;
}

// =============================================================================
// Main
// =============================================================================
int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Phase 9a: Enhanced Absurdity System" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    test_fuzzy_interval_basics();
    test_stochastic_evolution();
    test_neighborhood_evolution();
    test_fusion();
    test_uncertainty_comparison();
    test_absurdity_field();
    test_utilities();
    
    std::cout << "========================================" << std::endl;
    std::cout << "All tests passed!" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return 0;
}
