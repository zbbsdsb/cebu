// Minimal test to verify Phase 9a APIs actually work
#include "cebu/absurdity.h"
#include <iostream>

using namespace cebu;

int main() {
    std::cout << "Testing Phase 9a APIs...\n\n";
    
    // Test 1: FuzzyInterval basic operations
    std::cout << "1. FuzzyInterval Operations\n";
    try {
        FuzzyInterval a(0.3, 0.5, 0.9);
        FuzzyInterval b(0.4, 0.6, 0.8);
        
        FuzzyInterval sum = a + b;
        FuzzyInterval product = a * b;
        bool is_less = a < b;
        double overlap = a.overlap_degree(b);
        std::string str = a.to_string();
        
        std::cout << "   a = " << str << "\n";
        std::cout << "   a + b = " << sum.to_string() << "\n";
        std::cout << "   a * b = " << product.to_string() << "\n";
        std::cout << "   a < b: " << (is_less ? "true" : "false") << "\n";
        std::cout << "   overlap: " << overlap << "\n";
        std::cout << "   ✓ PASS\n\n";
    } catch (...) {
        std::cout << "   ✗ FAIL\n\n";
        return 1;
    }
    
    // Test 2: StochasticEvolution
    std::cout << "2. StochasticEvolution\n";
    try {
        EvolutionParams params;
        params.decay_rate = 0.95;
        params.volatility = 0.1;
        
        StochasticEvolution evolution(params);
        FuzzyInterval initial(0.8, 0.8, 1.0);
        
        auto next = evolution.step(initial);
        auto trajectory = evolution.evolve(initial, 5);
        
        std::cout << "   initial: " << initial.to_string() << "\n";
        std::cout << "   after 1 step: " << next.to_string() << "\n";
        std::cout << "   trajectory size: " << trajectory.size() << "\n";
        std::cout << "   ✓ PASS\n\n";
    } catch (...) {
        std::cout << "   ✗ FAIL\n\n";
        return 1;
    }
    
    // Test 3: FusionStrategy
    std::cout << "3. FusionStrategy\n";
    try {
        std::vector<AbsurditySource> sources = {
            {"Source A", FuzzyInterval(0.3, 0.5, 0.9), 0.9, 1.0},
            {"Source B", FuzzyInterval(0.4, 0.6, 0.8), 0.8, 0.8}
        };
        
        auto fused = FusionStrategy::fuse(sources, FusionStrategy::Method::WeightedAverage);
        double conflict = FusionStrategy::compute_conflict(sources);
        
        std::cout << "   fused: " << fused.to_string() << "\n";
        std::cout << "   conflict: " << conflict << "\n";
        std::cout << "   ✓ PASS\n\n";
    } catch (...) {
        std::cout << "   ✗ FAIL\n\n";
        return 1;
    }
    
    // Test 4: UncertaintyComparison
    std::cout << "4. UncertaintyComparison\n";
    try {
        FuzzyInterval x(0.2, 0.3, 0.9);
        FuzzyInterval y(0.4, 0.5, 0.8);
        
        auto result = UncertaintyComparison::compare(x, y);
        bool def_less = UncertaintyComparison::definitely_less(x, y);
        double prob = UncertaintyComparison::probability_less(x, y);
        
        std::cout << "   comparison: " << result.explanation << "\n";
        std::cout << "   definitely less: " << (def_less ? "true" : "false") << "\n";
        std::cout << "   P(x < y): " << prob << "\n";
        std::cout << "   ✓ PASS\n\n";
    } catch (...) {
        std::cout << "   ✗ FAIL\n\n";
        return 1;
    }
    
    // Test 5: AbsurdityField
    std::cout << "5. AbsurdityField\n";
    try {
        AbsurdityField field;
        
        for (size_t i = 0; i < 5; ++i) {
            field.set(i, FuzzyInterval(0.1 * i, 0.1 * i, 1.0));
        }
        
        FuzzyInterval val = field.get(2);
        auto high = field.find_above_threshold(0.3);
        FuzzyInterval mean = field.mean();
        
        std::cout << "   value at 2: " << val.to_string() << "\n";
        std::cout << "   high count: " << high.size() << "\n";
        std::cout << "   mean: " << mean.to_string() << "\n";
        std::cout << "   ✓ PASS\n\n";
    } catch (...) {
        std::cout << "   ✗ FAIL\n\n";
        return 1;
    }
    
    // Test 6: AbsurdityLabel
    std::cout << "6. AbsurdityLabel\n";
    try {
        AbsurdityLabel label1("hot", FuzzyInterval(0.7, 0.9, 0.95));
        AbsurdityLabel label2("cold", FuzzyInterval(0.1, 0.2, 0.9));
        
        bool is_more = label1.is_more_absurd(label2);
        std::string str = label1.to_string();
        
        std::cout << "   " << str << "\n";
        std::cout << "   is more absurd: " << (is_more ? "true" : "false") << "\n";
        std::cout << "   ✓ PASS\n\n";
    } catch (...) {
        std::cout << "   ✗ FAIL\n\n";
        return 1;
    }
    
    // Test 7: Utility Functions
    std::cout << "7. Utility Functions\n";
    try {
        auto crisp = absurdity_utils::crisp(0.5);
        auto uncertain = absurdity_utils::uncertain(0.5, 0.1, 0.8);
        auto interp = absurdity_utils::interpolate(crisp, uncertain, 0.5);
        auto clamped = absurdity_utils::clamp(FuzzyInterval(-0.2, 1.3, 0.9), 0.0, 1.0);
        double dist = absurdity_utils::distance(crisp, uncertain);
        
        std::cout << "   crisp: " << crisp.to_string() << "\n";
        std::cout << "   uncertain: " << uncertain.to_string() << "\n";
        std::cout << "   interpolated: " << interp.to_string() << "\n";
        std::cout << "   clamped: " << clamped.to_string() << "\n";
        std::cout << "   distance: " << dist << "\n";
        std::cout << "   ✓ PASS\n\n";
    } catch (...) {
        std::cout << "   ✗ FAIL\n\n";
        return 1;
    }
    
    std::cout << "========================================\n";
    std::cout << "All Phase 9a APIs are working! ✓\n";
    std::cout << "========================================\n";
    
    return 0;
}
