#include "cebu/absurdity.h"
#include <iostream>
#include <iomanip>

using namespace cebu;

void print_separator(const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << title << std::endl;
    std::cout << std::string(60, '=') << std::endl;
}

void demo_fuzzy_intervals() {
    print_separator("Demo 1: Fuzzy Interval Numbers");
    
    // Create different types of intervals
    FuzzyInterval certain(0.5, 0.5, 1.0);          // Completely certain
    FuzzyInterval uncertain(0.3, 0.7, 0.6);       // Uncertain with wide range
    FuzzyInterval vague(0.2, 0.8, 0.3);           // Very vague (low confidence)
    
    std::cout << "Certain:   " << certain.to_string() << std::endl;
    std::cout << "Uncertain: " << uncertain.to_string() << std::endl;
    std::cout << "Vague:     " << vague.to_string() << std::endl;
    
    // Arithmetic operations
    std::cout << "\nArithmetic:" << std::endl;
    FuzzyInterval a(0.2, 0.4, 0.9);
    FuzzyInterval b(0.3, 0.6, 0.8);
    
    std::cout << "a = " << a.to_string() << std::endl;
    std::cout << "b = " << b.to_string() << std::endl;
    std::cout << "a + b = " << (a + b).to_string() << std::endl;
    std::cout << "a * b = " << (a * b).to_string() << std::endl;
    std::cout << "a * 2 = " << (a * 2.0).to_string() << std::endl;
    
    // Set operations
    std::cout << "\nSet operations:" << std::endl;
    std::cout << "Intersection: " << a.intersect(b).to_string() << std::endl;
    std::cout << "Union:        " << a.union_with(b).to_string() << std::endl;
    std::cout << "Overlap:      " << a.overlap_degree(b) << std::endl;
}

void demo_stochastic_evolution() {
    print_separator("Demo 2: Stochastic Evolution");
    
    // Configure evolution parameters
    EvolutionParams params;
    params.decay_rate = 0.97;
    params.volatility = 0.08;
    params.coupling_strength = 0.25;
    params.diffusion_rate = 0.05;
    params.enable_levy_jumps = true;
    
    StochasticEvolution evolution(params);
    
    // Start with a crisp value
    FuzzyInterval initial(0.8, 0.8, 1.0);
    std::cout << "Initial absurdity: " << initial.to_string() << std::endl;
    std::cout << "Decay rate: " << params.decay_rate << std::endl;
    std::cout << "Volatility: " << params.volatility << std::endl;
    
    // Evolve over time
    std::cout << "\nEvolution over 20 steps:" << std::endl;
    auto trajectory = evolution.evolve(initial, 20);
    
    for (size_t i = 0; i < trajectory.size(); ++i) {
        if (i % 4 == 0 || i == trajectory.size() - 1) {
            const auto& val = trajectory[i];
            std::cout << "  Step " << std::setw(2) << i << ": "
                      << val.to_string()
                      << " (uncertainty=" << std::fixed << std::setprecision(3)
                      << val.width() << ")" << std::endl;
        }
    }
    
    // Demonstrate neighborhood evolution
    std::cout << "\nNeighborhood evolution (3x3 grid):" << std::endl;
    
    // Create 3x3 grid of absurdity values
    std::vector<FuzzyInterval> grid(9);
    for (size_t i = 0; i < 9; ++i) {
        // Create gradient from high (top-left) to low (bottom-right)
        double row = static_cast<double>(i / 3);
        double col = static_cast<double>(i % 3);
        double val = 0.9 - 0.1 * (row + col);
        grid[i] = FuzzyInterval(val, val, 1.0);
    }
    
    // Print initial grid
    std::cout << "  Initial:" << std::endl;
    for (int row = 0; row < 3; ++row) {
        std::cout << "    ";
        for (int col = 0; col < 3; ++col) {
            size_t idx = row * 3 + col;
            std::cout << std::fixed << std::setprecision(2) << grid[idx].midpoint() << " ";
        }
        std::cout << std::endl;
    }
    
    // Build adjacency (4-connected neighbors)
    std::vector<std::vector<size_t>> adj(9);
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            size_t idx = r * 3 + c;
            std::vector<size_t> neighbors;
            if (r > 0) neighbors.push_back((r-1) * 3 + c);
            if (r < 2) neighbors.push_back((r+1) * 3 + c);
            if (c > 0) neighbors.push_back(r * 3 + (c-1));
            if (c < 2) neighbors.push_back(r * 3 + (c+1));
            adj[idx] = neighbors;
        }
    }
    
    // Evolve
    for (int step = 0; step < 3; ++step) {
        grid = evolution.evolve_neighborhood(grid, adj, 1.0);
        
        std::cout << "  After step " << step + 1 << ":" << std::endl;
        for (int row = 0; row < 3; ++row) {
            std::cout << "    ";
            for (int col = 0; col < 3; ++col) {
                size_t idx = row * 3 + col;
                std::cout << std::fixed << std::setprecision(2) << grid[idx].midpoint() << " ";
            }
            std::cout << std::endl;
        }
    }
}

void demo_fusion() {
    print_separator("Demo 3: Multi-Source Fusion");
    
    // Simulate multiple observers estimating absurdity
    std::vector<AbsurditySource> sources = {
        {"Expert 1", FuzzyInterval(0.4, 0.6, 0.9), 0.95, 1.0, "Senior researcher"},
        {"Expert 2", FuzzyInterval(0.3, 0.5, 0.8), 0.85, 0.9, "Domain specialist"},
        {"Sensor A", FuzzyInterval(0.45, 0.55, 0.7), 0.7, 0.6, "Automated measurement"},
        {"Historical", FuzzyInterval(0.35, 0.65, 0.6), 0.6, 0.4, "Past data"},
        {"Intuition", FuzzyInterval(0.2, 0.8, 0.4), 0.4, 0.2, "Expert intuition"}
    };
    
    std::cout << "Sources:" << std::endl;
    for (const auto& src : sources) {
        std::cout << "  " << std::setw(10) << src.name << ": "
                  << src.value.to_string()
                  << " (rel=" << src.reliability << ", w=" << src.weight << ")" << std::endl;
    }
    
    // Analyze conflict
    double conflict = FusionStrategy::compute_conflict(sources);
    std::cout << "\nConflict analysis:" << std::endl;
    std::cout << "  Overall conflict: " << std::fixed << std::setprecision(3) << conflict << std::endl;
    
    auto method = FusionStrategy::select_method(sources);
    std::cout << "  Recommended method: ";
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
    std::cout << "  Adaptive:          " << FusionStrategy::fuse(sources, FusionStrategy::Method::Adaptive).to_string() << std::endl;
    
    // Progressive fusion
    std::cout << "\nProgressive fusion (adding sources one by one):" << std::endl;
    std::vector<AbsurditySource> progressive;
    for (size_t i = 0; i < sources.size(); ++i) {
        progressive.push_back(sources[i]);
        auto fused = FusionStrategy::fuse(progressive, FusionStrategy::Method::Adaptive);
        std::cout << "  After " << std::setw(2) << (i + 1) << " sources: " << fused.to_string() << std::endl;
    }
}

void demo_uncertainty_comparison() {
    print_separator("Demo 4: Uncertainty-Aware Comparison");
    
    // Create intervals with different uncertainty levels
    FuzzyInterval crisp_high(0.8, 0.8, 1.0);
    FuzzyInterval crisp_low(0.2, 0.2, 1.0);
    FuzzyInterval uncertain_mid(0.4, 0.6, 0.7);
    FuzzyInterval vague_wide(0.1, 0.9, 0.3);
    FuzzyInterval overlapping(0.3, 0.5, 0.9);
    
    std::cout << "Intervals to compare:" << std::endl;
    std::cout << "  crisp_high:     " << crisp_high.to_string() << std::endl;
    std::cout << "  crisp_low:      " << crisp_low.to_string() << std::endl;
    std::cout << "  uncertain_mid:  " << uncertain_mid.to_string() << std::endl;
    std::cout << "  vague_wide:     " << vague_wide.to_string() << std::endl;
    std::cout << "  overlapping:    " << overlapping.to_string() << std::endl;
    
    // Compare various pairs
    std::cout << "\nDetailed comparisons:" << std::endl;
    
    auto print_comparison = [&](const FuzzyInterval& a, const FuzzyInterval& b) {
        auto result = UncertaintyComparison::compare(a, b);
        std::cout << "  " << a.to_string() << "\n    vs " << b.to_string() << "\n    → "
                  << result.explanation << " (confidence=" << result.confidence << ")" << std::endl;
        std::cout << "    P(a < b) = " << UncertaintyComparison::probability_less(a, b) << std::endl;
    };
    
    print_comparison(crisp_high, crisp_low);
    print_comparison(uncertain_mid, overlapping);
    print_comparison(vague_wide, uncertain_mid);
    
    // Sorting with uncertainty
    std::cout << "\nSorting intervals by midpoint:" << std::endl;
    std::vector<FuzzyInterval> intervals = {crisp_high, crisp_low, uncertain_mid, vague_wide, overlapping};
    UncertaintyComparison::sort_with_uncertainty(intervals);
    
    for (const auto& val : intervals) {
        std::cout << "  " << val.to_string() << std::endl;
    }
}

void demo_absurdity_field() {
    print_separator("Demo 5: Absurdity Field Operations");
    
    AbsurdityField field;
    
    // Initialize field with a pattern
    std::cout << "Initializing 5x5 field with gradient pattern:" << std::endl;
    for (int row = 0; row < 5; ++row) {
        for (int col = 0; col < 5; ++col) {
            size_t id = row * 5 + col;
            double val = 0.1 * (row + col);
            field.set(id, FuzzyInterval(val, val, 1.0));
        }
    }
    
    // Print field
    std::cout << "\nInitial field:" << std::endl;
    for (int row = 0; row < 5; ++row) {
        std::cout << "  ";
        for (int col = 0; col < 5; ++col) {
            size_t id = row * 5 + col;
            double val = field.get(id).midpoint();
            std::cout << std::fixed << std::setprecision(1) << val << " ";
        }
        std::cout << std::endl;
    }
    
    // Statistics
    std::cout << "\nField statistics:" << std::endl;
    std::cout << "  Mean:     " << field.mean().to_string() << std::endl;
    std::cout << "  Variance: " << field.variance().to_string() << std::endl;
    std::cout << "  Min:      " << field.min().to_string() << std::endl;
    std::cout << "  Max:      " << field.max().to_string() << std::endl;
    
    // Query operations
    auto high_absurdity = field.find_above_threshold(0.6);
    auto low_absurdity = field.find_below_threshold(0.4);
    auto top_5 = field.find_extremes(5);
    
    std::cout << "\nQuery results:" << std::endl;
    std::cout << "  Above 0.6: ";
    for (size_t id : high_absurdity) std::cout << id << " ";
    std::cout << "\n  Below 0.4: ";
    for (size_t id : low_absurdity) std::cout << id << " ";
    std::cout << "\n  Top 5: ";
    for (size_t id : top_5) std::cout << id << " ";
    std::cout << std::endl;
    
    // Diffusion
    field.diffuse(0.2, 3);
    std::cout << "\nAfter diffusion (rate=0.2, 3 iterations):" << std::endl;
    for (int row = 0; row < 5; ++row) {
        std::cout << "  ";
        for (int col = 0; col < 5; ++col) {
            size_t id = row * 5 + col;
            double val = field.get(id).midpoint();
            std::cout << std::fixed << std::setprecision(2) << val << " ";
        }
        std::cout << std::endl;
    }
    
    // Stochastic evolution
    EvolutionParams params;
    params.decay_rate = 0.99;
    params.volatility = 0.05;
    StochasticEvolution evolution(params);
    
    field.evolve(evolution, 1.0);
    std::cout << "\nAfter stochastic evolution (1 step):" << std::endl;
    for (int row = 0; row < 5; ++row) {
        std::cout << "  ";
        for (int col = 0; col < 5; ++col) {
            size_t id = row * 5 + col;
            const auto& val = field.get(id);
            std::cout << std::fixed << std::setprecision(2) << val.midpoint() << " ";
        }
        std::cout << std::endl;
    }
}

void demo_absurdity_labels() {
    print_separator("Demo 6: Absurdity Labels");
    
    // Create labeled absurdities
    AbsurdityLabel hot("hot", FuzzyInterval(0.7, 0.9, 0.95));
    AbsurdityLabel warm("warm", FuzzyInterval(0.4, 0.6, 0.9));
    AbsurdityLabel cool("cool", FuzzyInterval(0.2, 0.4, 0.85));
    AbsurdityLabel cold("cold", FuzzyInterval(0.0, 0.2, 0.95));
    
    std::cout << "Labeled absurdities:" << std::endl;
    std::cout << "  " << hot.to_string() << std::endl;
    std::cout << "  " << warm.to_string() << std::endl;
    std::cout << "  " << cool.to_string() << std::endl;
    std::cout << "  " << cold.to_string() << std::endl;
    
    // Comparisons
    std::cout << "\nComparisons:" << std::endl;
    std::cout << "  hot is more absurd than warm: " << (hot.is_more_absurd(warm) ? "true" : "false") << std::endl;
    std::cout << "  cool is less absurd than hot: " << (cool.is_less_absurd(hot) ? "true" : "false") << std::endl;
    
    auto result = hot.compare_absurdity(warm);
    std::cout << "  hot vs warm: " << result.explanation 
              << " (confidence=" << result.confidence << ")" << std::endl;
}

void demo_complete_workflow() {
    print_separator("Demo 7: Complete Workflow");
    
    std::cout << "Scenario: Estimating absurdity for a story event" << std::endl;
    
    // Step 1: Multiple observers provide initial estimates
    std::cout << "\nStep 1: Gather estimates from multiple sources" << std::endl;
    std::vector<AbsurditySource> estimates = {
        {"Narrator", FuzzyInterval(0.6, 0.8, 0.8), 0.9, 1.0, "Storyteller perspective"},
        {"Character A", FuzzyInterval(0.4, 0.6, 0.7), 0.7, 0.7, "First-person view"},
        {"Historical Context", FuzzyInterval(0.5, 0.7, 0.6), 0.8, 0.8, "World background"},
        {"Reader Feedback", FuzzyInterval(0.3, 0.5, 0.5), 0.5, 0.5, "Audience reaction"}
    };
    
    for (const auto& src : estimates) {
        std::cout << "  " << src.name << ": " << src.value.to_string() << std::endl;
    }
    
    // Step 2: Fuse estimates
    std::cout << "\nStep 2: Fuse estimates using adaptive method" << std::endl;
    auto fused = FusionStrategy::fuse(estimates, FusionStrategy::Method::Adaptive);
    std::cout << "  Fused absurdity: " << fused.to_string() << std::endl;
    
    // Step 3: Create label
    std::cout << "\nStep 3: Create labeled absurdity" << std::endl;
    AbsurdityLabel event_label("The Awakening", fused);
    std::cout << "  " << event_label.to_string() << std::endl;
    
    // Step 4: Evolve absurdity over time
    std::cout << "\nStep 4: Evolve absurdity over story time" << std::endl;
    EvolutionParams params;
    params.decay_rate = 0.98;
    params.volatility = 0.05;
    StochasticEvolution evolution(params);
    
    auto trajectory = evolution.evolve(fused, 10);
    std::cout << "  Time evolution:" << std::endl;
    for (size_t i = 0; i < trajectory.size(); ++i) {
        if (i % 2 == 0 || i == trajectory.size() - 1) {
            std::cout << "    t=" << std::setw(2) << i << ": " << trajectory[i].to_string() << std::endl;
        }
    }
    
    // Step 5: Apply to a field
    std::cout << "\nStep 5: Apply to absurdity field" << std::endl;
    AbsurdityField field;
    for (size_t i = 0; i < 9; ++i) {
        field.set(i, fused);  // Start with same absurdity
    }
    
    std::cout << "  Initial field (3x3):" << std::endl;
    for (int row = 0; row < 3; ++row) {
        std::cout << "    ";
        for (int col = 0; col < 3; ++col) {
            size_t id = row * 3 + col;
            std::cout << std::fixed << std::setprecision(2) << field.get(id).midpoint() << " ";
        }
        std::cout << std::endl;
    }
    
    // Step 6: Add spatial variation
    std::cout << "\nStep 6: Add spatial variation" << std::endl;
    field.set(0, FuzzyInterval(0.9, 0.95, 0.95));  // High absurdity corner
    field.set(4, FuzzyInterval(0.4, 0.5, 0.9));    // Medium center
    field.set(8, FuzzyInterval(0.1, 0.15, 0.95));   // Low absurdity corner
    
    std::cout << "  After adding variation:" << std::endl;
    for (int row = 0; row < 3; ++row) {
        std::cout << "    ";
        for (int col = 0; col < 3; ++col) {
            size_t id = row * 3 + col;
            std::cout << std::fixed << std::setprecision(2) << field.get(id).midpoint() << " ";
        }
        std::cout << std::endl;
    }
    
    // Step 7: Diffuse
    std::cout << "\nStep 7: Diffuse to smooth variations" << std::endl;
    field.diffuse(0.3, 5);
    
    std::cout << "  After diffusion:" << std::endl;
    for (int row = 0; row < 3; ++row) {
        std::cout << "    ";
        for (int col = 0; col < 3; ++col) {
            size_t id = row * 3 + col;
            std::cout << std::fixed << std::setprecision(2) << field.get(id).midpoint() << " ";
        }
        std::cout << std::endl;
    }
    
    std::cout << "\nComplete workflow finished!" << std::endl;
}

int main() {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════╗\n";
    std::cout << "║   Phase 9a: Enhanced Absurdity System - Demo          ║\n";
    std::cout << "╚════════════════════════════════════════════════════════╝\n";
    
    demo_fuzzy_intervals();
    demo_stochastic_evolution();
    demo_fusion();
    demo_uncertainty_comparison();
    demo_absurdity_field();
    demo_absurdity_labels();
    demo_complete_workflow();
    
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════╗\n";
    std::cout << "║   Demo Complete!                                        ║\n";
    std::cout << "╚════════════════════════════════════════════════════════╝\n";
    std::cout << std::endl;
    
    return 0;
}
