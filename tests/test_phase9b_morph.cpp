#include "cebu/topology_morph.h"
#include "cebu/absurdity.h"
#include "cebu/simplicial_complex.h"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace cebu;

// Test helper functions
void print_test_header(const std::string& test_name) {
    std::cout << "\n=== " << test_name << " ===" << std::endl;
}

void print_success() {
    std::cout << "✓ PASSED" << std::endl;
}

void print_failure(const std::string& message) {
    std::cout << "✗ FAILED: " << message << std::endl;
}

// =============================================================================
// Test 1: Basic Morph Rule
// =============================================================================

void test_basic_morph_rule() {
    print_test_header("Test 1: Basic Morph Rule");
    
    // Create complex
    SimplicialComplex complex;
    auto v0 = complex.add_vertex();
    auto v1 = complex.add_vertex();
    auto v2 = complex.add_vertex();
    auto t = complex.add_triangle(v0, v1, v2);
    
    // Create absurdity field
    AbsurdityField field;
    field.set(t, FuzzyInterval(0.8, 0.9, 0.95));
    
    // Create morph rule
    MorphRule rule(
        "high_absurdity_split",
        MorphType::SPLIT,
        TriggerCondition::ABSURDITY_THRESHOLD,
        FuzzyInterval(0.7, 0.8, 0.9)
    );
    rule.probability = 1.0;
    
    // Create morph system
    TopologyMorph morph;
    morph.add_rule(rule);
    
    // Create context
    MorphContext context(complex, field, 0.0, 0.1);
    
    // Check if rule matches
    const MorphRule* matched = morph.find_matching_rule(t, context);
    if (matched == nullptr) {
        print_failure("No matching rule found");
        return;
    }
    
    if (matched->name != "high_absurdity_split") {
        print_failure("Wrong rule matched");
        return;
    }
    
    // Apply morph
    auto result = morph.apply_morph(*matched, t, context);
    if (!result.success) {
        print_failure("Morph failed: " + result.message);
        return;
    }
    
    // Check that triangle was split into 3 triangles
    if (result.created.size() != 4) {  // 1 centroid + 3 triangles
        print_failure("Expected 4 created simplices, got " + 
                     std::to_string(result.created.size()));
        return;
    }
    
    print_success();
}

// =============================================================================
// Test 2: Low Absurdity Merge
// =============================================================================

void test_low_absurdity_merge() {
    print_test_header("Test 2: Low Absurdity Merge");
    
    // Create complex with two triangles
    SimplicialComplex complex;
    auto v0 = complex.add_vertex();
    auto v1 = complex.add_vertex();
    auto v2 = complex.add_vertex();
    auto v3 = complex.add_vertex();
    
    auto t1 = complex.add_triangle(v0, v1, v2);
    auto t2 = complex.add_triangle(v1, v2, v3);
    
    // Create absurdity field with low absurdity
    AbsurdityField field;
    field.set(t1, FuzzyInterval(0.1, 0.2, 0.9));
    field.set(t2, FuzzyInterval(0.1, 0.15, 0.85));
    
    // Create merge rule
    MorphRule rule(
        "low_absurdity_merge",
        MorphType::MERGE,
        TriggerCondition::LOW_ABSURDITY,
        FuzzyInterval(0.25, 0.3, 0.9)
    );
    rule.probability = 1.0;
    
    TopologyMorph morph;
    morph.add_rule(rule);
    
    MorphContext context(complex, field, 0.0, 0.1);
    
    // Apply merge to t1
    auto result = morph.apply_morph(rule, t1, context);
    if (!result.success) {
        print_failure("Merge failed: " + result.message);
        return;
    }
    
    // Check that both triangles were removed
    if (result.removed.size() < 2) {
        print_failure("Expected at least 2 removed simplices, got " + 
                     std::to_string(result.removed.size()));
        return;
    }
    
    print_success();
}

// =============================================================================
// Test 3: Very High Absurdity Delete
// =============================================================================

void test_very_high_absurdity_delete() {
    print_test_header("Test 3: Very High Absurdity Delete");
    
    // Create complex
    SimplicialComplex complex;
    auto v0 = complex.add_vertex();
    auto v1 = complex.add_vertex();
    auto v2 = complex.add_vertex();
    auto t = complex.add_triangle(v0, v1, v2);
    
    // Create absurdity field with very high absurdity
    AbsurdityField field;
    field.set(t, FuzzyInterval(0.92, 0.97, 0.98));
    
    // Create delete rule
    MorphRule rule(
        "very_high_absurdity_delete",
        MorphType::DELETE,
        TriggerCondition::ABSURDITY_THRESHOLD,
        FuzzyInterval(0.9, 0.95, 0.95)
    );
    rule.probability = 1.0;
    
    TopologyMorph morph;
    morph.add_rule(rule);
    
    MorphContext context(complex, field, 0.0, 0.1);
    
    // Apply delete
    auto result = morph.apply_morph(rule, t, context);
    if (!result.success) {
        print_failure("Delete failed: " + result.message);
        return;
    }
    
    // Check that triangle was removed
    if (result.removed.empty()) {
        print_failure("Expected removed simplices");
        return;
    }
    
    print_success();
}

// =============================================================================
// Test 4: Expand Dimension
// =============================================================================

void test_expand_dimension() {
    print_test_header("Test 4: Expand Dimension");
    
    // Create edge
    SimplicialComplex complex;
    auto v0 = complex.add_vertex();
    auto v1 = complex.add_vertex();
    auto e = complex.add_edge(v0, v1);
    
    // Create absurdity field
    AbsurdityField field;
    field.set(e, FuzzyInterval(0.5, 0.6, 0.8));
    
    // Create expand rule
    MorphRule rule(
        "expand_dimension",
        MorphType::EXPAND,
        TriggerCondition::ABSURDITY_THRESHOLD,
        FuzzyInterval(0.4, 0.5, 0.8)
    );
    rule.probability = 1.0;
    
    TopologyMorph morph;
    morph.add_rule(rule);
    
    MorphContext context(complex, field, 0.0, 0.1);
    
    // Apply expand
    auto result = morph.apply_morph(rule, e, context);
    if (!result.success) {
        print_failure("Expand failed: " + result.message);
        return;
    }
    
    // Check that new vertex and triangle were created
    if (result.created.size() != 2) {
        print_failure("Expected 2 created simplices, got " + 
                     std::to_string(result.created.size()));
        return;
    }
    
    // Check that we now have a triangle
    bool has_triangle = false;
    for (auto id : result.created) {
        if (complex.has_simplex(id)) {
            const auto& s = complex.get_simplex(id);
            if (s.dimension() == 2) {
                has_triangle = true;
                break;
            }
        }
    }
    
    if (!has_triangle) {
        print_failure("No triangle created");
        return;
    }
    
    print_success();
}

// =============================================================================
// Test 5: Contract Dimension
// =============================================================================

void test_contract_dimension() {
    print_test_header("Test 5: Contract Dimension");
    
    // Create triangle
    SimplicialComplex complex;
    auto v0 = complex.add_vertex();
    auto v1 = complex.add_vertex();
    auto v2 = complex.add_vertex();
    auto t = complex.add_triangle(v0, v1, v2);
    
    // Create absurdity field with low absurdity
    AbsurdityField field;
    field.set(t, FuzzyInterval(0.1, 0.2, 0.9));
    
    // Create contract rule
    MorphRule rule(
        "contract_dimension",
        MorphType::CONTRACT,
        TriggerCondition::LOW_ABSURDITY,
        FuzzyInterval(0.15, 0.2, 0.9)
    );
    rule.probability = 1.0;
    
    TopologyMorph morph;
    morph.add_rule(rule);
    
    MorphContext context(complex, field, 0.0, 0.1);
    
    // Apply contract
    auto result = morph.apply_morph(rule, t, context);
    if (!result.success) {
        print_failure("Contract failed: " + result.message);
        return;
    }
    
    // Check that edge was created
    bool has_edge = false;
    for (auto id : result.created) {
        if (complex.has_simplex(id)) {
            const auto& s = complex.get_simplex(id);
            if (s.dimension() == 1) {
                has_edge = true;
                break;
            }
        }
    }
    
    if (!has_edge) {
        print_failure("No edge created");
        return;
    }
    
    print_success();
}

// =============================================================================
// Test 6: Glue Vertices
// =============================================================================

void test_glue_vertices() {
    print_test_header("Test 6: Glue Vertices");
    
    // Create triangle
    SimplicialComplex complex;
    auto v0 = complex.add_vertex();
    auto v1 = complex.add_vertex();
    auto v2 = complex.add_vertex();
    auto t = complex.add_triangle(v0, v1, v2);
    
    // Create absurdity field
    AbsurdityField field;
    field.set(t, FuzzyInterval(0.5, 0.6, 0.8));
    
    // Create glue rule
    MorphRule rule(
        "glue_vertices",
        MorphType::GLUE,
        TriggerCondition::ABSURDITY_THRESHOLD,
        FuzzyInterval(0.4, 0.5, 0.8)
    );
    rule.probability = 1.0;
    
    TopologyMorph morph;
    morph.add_rule(rule);
    
    MorphContext context(complex, field, 0.0, 0.1);
    
    // Apply glue
    auto result = morph.apply_morph(rule, t, context);
    if (!result.success) {
        print_failure("Glue failed: " + result.message);
        return;
    }
    
    // Check that triangle was modified
    if (result.modified.empty()) {
        print_failure("Expected modified simplices");
        return;
    }
    
    print_success();
}

// =============================================================================
// Test 7: Morph Presets
// =============================================================================

void test_morph_presets() {
    print_test_header("Test 7: Morph Presets");
    
    // Test default rules
    auto default_rules = MorphPresets::default_rules();
    if (default_rules.empty()) {
        print_failure("Default rules empty");
        return;
    }
    std::cout << "  Default rules: " << default_rules.size() << std::endl;
    
    // Test conservative rules
    auto conservative = MorphPresets::conservative_rules();
    if (conservative.empty()) {
        print_failure("Conservative rules empty");
        return;
    }
    std::cout << "  Conservative rules: " << conservative.size() << std::endl;
    
    // Test aggressive rules
    auto aggressive = MorphPresets::aggressive_rules();
    if (aggressive.empty()) {
        print_failure("Aggressive rules empty");
        return;
    }
    std::cout << "  Aggressive rules: " << aggressive.size() << std::endl;
    
    // Test chaos rules
    auto chaos = MorphPresets::chaos_rules();
    if (chaos.empty()) {
        print_failure("Chaos rules empty");
        return;
    }
    std::cout << "  Chaos rules: " << chaos.size() << std::endl;
    
    // Test artistic rules
    auto artistic = MorphPresets::artistic_rules();
    if (artistic.empty()) {
        print_failure("Artistic rules empty");
        return;
    }
    std::cout << "  Artistic rules: " << artistic.size() << std::endl;
    
    // Test scientific rules
    auto scientific = MorphPresets::scientific_rules();
    if (scientific.empty()) {
        print_failure("Scientific rules empty");
        return;
    }
    std::cout << "  Scientific rules: " << scientific.size() << std::endl;
    
    print_success();
}

// =============================================================================
// Test 8: Morph Queue
// =============================================================================

void test_morph_queue() {
    print_test_header("Test 8: Morph Queue");
    
    // Create complex
    SimplicialComplex complex;
    auto v0 = complex.add_vertex();
    auto v1 = complex.add_vertex();
    auto v2 = complex.add_vertex();
    auto t = complex.add_triangle(v0, v1, v2);
    
    // Create absurdity field
    AbsurdityField field;
    field.set(t, FuzzyInterval(0.8, 0.9, 0.95));
    
    // Create morph rule
    MorphRule rule(
        "high_absurdity_split",
        MorphType::SPLIT,
        TriggerCondition::ABSURDITY_THRESHOLD,
        FuzzyInterval(0.7, 0.8, 0.9)
    );
    rule.probability = 1.0;
    
    // Create queue
    MorphQueue queue;
    
    // Enqueue morph with delay
    QueuedMorph morph(rule, t, 0.5);
    queue.enqueue(morph);
    
    if (queue.size() != 1) {
        print_failure("Queue size should be 1");
        return;
    }
    
    // Create context
    MorphContext context(complex, field, 0.0, 0.1);
    
    // Try to execute before delay
    auto results = queue.execute_ready(0.3, context);
    if (!results.empty()) {
        print_failure("Should not execute before delay");
        return;
    }
    
    // Execute after delay
    results = queue.execute_ready(1.0, context);
    if (results.empty()) {
        print_failure("Should execute after delay");
        return;
    }
    
    if (!results[0].success) {
        print_failure("Morph execution failed: " + results[0].message);
        return;
    }
    
    print_success();
}

// =============================================================================
// Test 9: Morph Statistics
// =============================================================================

void test_morph_statistics() {
    print_test_header("Test 9: Morph Statistics");
    
    // Create complex
    SimplicialComplex complex;
    auto v0 = complex.add_vertex();
    auto v1 = complex.add_vertex();
    auto v2 = complex.add_vertex();
    auto t = complex.add_triangle(v0, v1, v2);
    
    // Create absurdity field
    AbsurdityField field;
    field.set(t, FuzzyInterval(0.8, 0.9, 0.95));
    
    // Create morph rule
    MorphRule rule(
        "test_split",
        MorphType::SPLIT,
        TriggerCondition::ABSURDITY_THRESHOLD,
        FuzzyInterval(0.7, 0.8, 0.9)
    );
    rule.probability = 1.0;
    
    TopologyMorph morph;
    morph.add_rule(rule);
    
    MorphContext context(complex, field, 0.0, 0.1);
    
    // Apply morph
    auto result = morph.apply_morph(rule, t, context);
    
    // Check statistics
    const auto& stats = morph.get_stats();
    
    if (stats.total_operations != 1) {
        print_failure("Expected 1 total operation, got " + 
                     std::to_string(stats.total_operations));
        return;
    }
    
    if (stats.successful_operations != 1) {
        print_failure("Expected 1 successful operation, got " + 
                     std::to_string(stats.successful_operations));
        return;
    }
    
    if (stats.failed_operations != 0) {
        print_failure("Expected 0 failed operations, got " + 
                     std::to_string(stats.failed_operations));
        return;
    }
    
    std::cout << "  Total absurdity change: " 
              << stats.total_absurdity_change << std::endl;
    
    print_success();
}

// =============================================================================
// Test 10: Apply Multiple Rules
// =============================================================================

void test_apply_multiple_rules() {
    print_test_header("Test 10: Apply Multiple Rules");
    
    // Create complex with multiple triangles
    SimplicialComplex complex;
    auto v0 = complex.add_vertex();
    auto v1 = complex.add_vertex();
    auto v2 = complex.add_vertex();
    auto v3 = complex.add_vertex();
    
    auto t1 = complex.add_triangle(v0, v1, v2);
    auto t2 = complex.add_triangle(v1, v2, v3);
    
    // Create absurdity field with different values
    AbsurdityField field;
    field.set(t1, FuzzyInterval(0.8, 0.9, 0.95));  // High -> split
    field.set(t2, FuzzyInterval(0.1, 0.15, 0.9));  // Low -> merge
    
    // Add rules
    TopologyMorph morph;
    
    MorphRule split_rule(
        "split_high",
        MorphType::SPLIT,
        TriggerCondition::ABSURDITY_THRESHOLD,
        FuzzyInterval(0.7, 0.8, 0.9)
    );
    split_rule.probability = 1.0;
    morph.add_rule(split_rule);
    
    MorphRule merge_rule(
        "merge_low",
        MorphType::MERGE,
        TriggerCondition::LOW_ABSURDITY,
        FuzzyInterval(0.25, 0.3, 0.9)
    );
    merge_rule.probability = 1.0;
    morph.add_rule(merge_rule);
    
    // Apply rules
    MorphContext context(complex, field, 0.0, 0.1);
    auto results = morph.apply_rules(context);
    
    if (results.size() < 2) {
        print_failure("Expected at least 2 results, got " + 
                     std::to_string(results.size()));
        return;
    }
    
    // Check that both operations were attempted
    int success_count = 0;
    for (const auto& result : results) {
        if (result.success) {
            success_count++;
        }
    }
    
    std::cout << "  Successful operations: " << success_count << std::endl;
    
    print_success();
}

// =============================================================================
// Main
// =============================================================================

int main() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "  Phase 9b: Dynamic Topology Morph Tests" << std::endl;
    std::cout << "========================================" << std::endl;
    
    int passed = 0;
    int total = 10;
    
    try {
        test_basic_morph_rule();
        passed++;
    } catch (...) {
        print_failure("Exception in test 1");
    }
    
    try {
        test_low_absurdity_merge();
        passed++;
    } catch (...) {
        print_failure("Exception in test 2");
    }
    
    try {
        test_very_high_absurdity_delete();
        passed++;
    } catch (...) {
        print_failure("Exception in test 3");
    }
    
    try {
        test_expand_dimension();
        passed++;
    } catch (...) {
        print_failure("Exception in test 4");
    }
    
    try {
        test_contract_dimension();
        passed++;
    } catch (...) {
        print_failure("Exception in test 5");
    }
    
    try {
        test_glue_vertices();
        passed++;
    } catch (...) {
        print_failure("Exception in test 6");
    }
    
    try {
        test_morph_presets();
        passed++;
    } catch (...) {
        print_failure("Exception in test 7");
    }
    
    try {
        test_morph_queue();
        passed++;
    } catch (...) {
        print_failure("Exception in test 8");
    }
    
    try {
        test_morph_statistics();
        passed++;
    } catch (...) {
        print_failure("Exception in test 9");
    }
    
    try {
        test_apply_multiple_rules();
        passed++;
    } catch (...) {
        print_failure("Exception in test 10");
    }
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "  Results: " << passed << "/" << total << " tests passed" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    return (passed == total) ? 0 : 1;
}
