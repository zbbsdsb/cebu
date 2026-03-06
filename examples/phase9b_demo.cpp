#include "cebu/topology_morph.h"
#include "cebu/absurdity.h"
#include "cebu/simplicial_complex.h"
#include <iostream>
#include <iomanip>
#include <sstream>

using namespace cebu;

// Demo helpers
void print_demo_header(const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "  " << title << std::endl;
    std::cout << std::string(60, '=') << std::endl;
}

void print_section_header(const std::string& title) {
    std::cout << "\n--- " << title << " ---" << std::endl;
}

void print_complex_stats(const SimplicialComplex& complex) {
    std::cout << "  Complex statistics:" << std::endl;
    std::cout << "    Vertices: " << complex.get_simplices_of_dimension(0).size() << std::endl;
    std::cout << "    Edges: " << complex.get_simplices_of_dimension(1).size() << std::endl;
    std::cout << "    Triangles: " << complex.get_simplices_of_dimension(2).size() << std::endl;
    std::cout << "    Total simplices: " << complex.get_simplices().size() << std::endl;
}

void print_morph_result(const MorphResult& result) {
    std::cout << "  Morph result:" << std::endl;
    std::cout << "    Success: " << (result.success ? "Yes" : "No") << std::endl;
    std::cout << "    Message: " << result.message << std::endl;
    std::cout << "    Created: " << result.created.size() << " simplices" << std::endl;
    std::cout << "    Modified: " << result.modified.size() << " simplices" << std::endl;
    std::cout << "    Removed: " << result.removed.size() << " simplices" << std::endl;
    std::cout << "    Absurdity change: " 
              << result.absurdity_before << " -> " 
              << result.absurdity_after << std::endl;
}

// =============================================================================
// Demo 1: Basic Split Operation
// =============================================================================

void demo_1_basic_split() {
    print_demo_header("Demo 1: Basic Split Operation");
    
    // Create a simple triangle
    SimplicialComplex complex;
    auto v0 = complex.add_vertex();
    auto v1 = complex.add_vertex();
    auto v2 = complex.add_vertex();
    auto t = complex.add_triangle(v0, v1, v2);
    
    std::cout << "\nInitial state:" << std::endl;
    print_complex_stats(complex);
    
    // Create absurdity field with high absurdity
    AbsurdityField field;
    field.set(t, FuzzyInterval(0.85, 0.95, 0.98));
    
    std::cout << "\nTriangle absurdity: " 
              << field.get(t).to_string() << std::endl;
    
    // Create split rule
    MorphRule rule(
        "high_absurdity_split",
        MorphType::SPLIT,
        TriggerCondition::ABSURDITY_THRESHOLD,
        FuzzyInterval(0.7, 0.8, 0.9)
    );
    rule.probability = 1.0;
    
    TopologyMorph morph;
    morph.add_rule(rule);
    
    MorphContext context(complex, field, 0.0, 0.1);
    
    // Apply split
    auto result = morph.apply_morph(rule, t, context);
    
    std::cout << "\nAfter split:" << std::endl;
    print_morph_result(result);
    print_complex_stats(complex);
}

// =============================================================================
// Demo 2: Low Absurdity Merge
// =============================================================================

void demo_2_low_absurdity_merge() {
    print_demo_header("Demo 2: Low Absurdity Merge");
    
    // Create two adjacent triangles
    SimplicialComplex complex;
    auto v0 = complex.add_vertex();
    auto v1 = complex.add_vertex();
    auto v2 = complex.add_vertex();
    auto v3 = complex.add_vertex();
    
    auto t1 = complex.add_triangle(v0, v1, v2);
    auto t2 = complex.add_triangle(v1, v2, v3);
    
    std::cout << "\nInitial state:" << std::endl;
    print_complex_stats(complex);
    
    // Create absurdity field with low absurdity
    AbsurdityField field;
    field.set(t1, FuzzyInterval(0.1, 0.15, 0.92));
    field.set(t2, FuzzyInterval(0.08, 0.12, 0.88));
    
    std::cout << "\nTriangle 1 absurdity: " 
              << field.get(t1).to_string() << std::endl;
    std::cout << "Triangle 2 absurdity: " 
              << field.get(t2).to_string() << std::endl;
    
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
    
    // Apply merge
    auto result = morph.apply_morph(rule, t1, context);
    
    std::cout << "\nAfter merge:" << std::endl;
    print_morph_result(result);
    print_complex_stats(complex);
}

// =============================================================================
// Demo 3: Expand and Contract
// =============================================================================

void demo_3_expand_contract() {
    print_demo_header("Demo 3: Expand and Contract Dimensions");
    
    // Create an edge
    SimplicialComplex complex;
    auto v0 = complex.add_vertex();
    auto v1 = complex.add_vertex();
    auto e = complex.add_edge(v0, v1);
    
    std::cout << "\nInitial state (edge):" << std::endl;
    print_complex_stats(complex);
    
    // Create absurdity field
    AbsurdityField field;
    field.set(e, FuzzyInterval(0.6, 0.7, 0.85));
    
    // Expand to triangle
    MorphRule expand_rule(
        "expand_to_triangle",
        MorphType::EXPAND,
        TriggerCondition::ABSURDITY_THRESHOLD,
        FuzzyInterval(0.5, 0.6, 0.8)
    );
    expand_rule.probability = 1.0;
    
    TopologyMorph morph;
    morph.add_rule(expand_rule);
    
    MorphContext context(complex, field, 0.0, 0.1);
    
    auto result1 = morph.apply_morph(expand_rule, e, context);
    
    std::cout << "\nAfter expand (to triangle):" << std::endl;
    print_morph_result(result1);
    print_complex_stats(complex);
    
    // Find the triangle and contract it
    SimplexID triangle_id = 0;
    for (auto id : result1.created) {
        if (complex.has_simplex(id)) {
            const auto& s = complex.get_simplex(id);
            if (s.dimension() == 2) {
                triangle_id = id;
                break;
            }
        }
    }
    
    if (triangle_id != 0) {
        field.set(triangle_id, FuzzyInterval(0.1, 0.15, 0.9));
        
        MorphRule contract_rule(
            "contract_to_edge",
            MorphType::CONTRACT,
            TriggerCondition::LOW_ABSURDITY,
            FuzzyInterval(0.15, 0.2, 0.9)
        );
        contract_rule.probability = 1.0;
        
        auto result2 = morph.apply_morph(contract_rule, triangle_id, context);
        
        std::cout << "\nAfter contract (back to edge):" << std::endl;
        print_morph_result(result2);
        print_complex_stats(complex);
    }
}

// =============================================================================
// Demo 4: Using Presets
// =============================================================================

void demo_4_presets() {
    print_demo_header("Demo 4: Using Morph Presets");
    
    // Create a grid of triangles
    SimplicialComplex complex;
    std::vector<VertexID> vertices;
    for (int i = 0; i < 4; ++i) {
        vertices.push_back(complex.add_vertex());
    }
    
    auto t1 = complex.add_triangle(vertices[0], vertices[1], vertices[2]);
    auto t2 = complex.add_triangle(vertices[1], vertices[3], vertices[2]);
    
    std::cout << "\nInitial state:" << std::endl;
    print_complex_stats(complex);
    
    // Create absurdity field with varying values
    AbsurdityField field;
    field.set(t1, FuzzyInterval(0.3, 0.4, 0.8));
    field.set(t2, FuzzyInterval(0.6, 0.7, 0.85));
    
    // Test different presets
    print_section_header("Conservative Preset");
    TopologyMorph conservative_morph;
    auto conservative_rules = MorphPresets::conservative_rules();
    for (const auto& rule : conservative_rules) {
        conservative_morph.add_rule(rule);
    }
    std::cout << "Loaded " << conservative_rules.size() << " conservative rules" << std::endl;
    
    MorphContext context(complex, field, 0.0, 0.1);
    auto conservative_results = conservative_morph.apply_rules(context);
    std::cout << "Applied " << conservative_results.size() << " operations" << std::endl;
    print_complex_stats(complex);
    
    print_section_header("Aggressive Preset");
    complex = SimplicialComplex();
    for (int i = 0; i < 4; ++i) {
        vertices.push_back(complex.add_vertex());
    }
    t1 = complex.add_triangle(vertices[0], vertices[1], vertices[2]);
    t2 = complex.add_triangle(vertices[1], vertices[3], vertices[2]);
    field.set(t1, FuzzyInterval(0.5, 0.6, 0.8));
    field.set(t2, FuzzyInterval(0.7, 0.8, 0.85));
    
    TopologyMorph aggressive_morph;
    auto aggressive_rules = MorphPresets::aggressive_rules();
    for (const auto& rule : aggressive_rules) {
        aggressive_morph.add_rule(rule);
    }
    std::cout << "Loaded " << aggressive_rules.size() << " aggressive rules" << std::endl;
    
    MorphContext context2(complex, field, 0.0, 0.1);
    auto aggressive_results = aggressive_morph.apply_rules(context2);
    std::cout << "Applied " << aggressive_results.size() << " operations" << std::endl;
    print_complex_stats(complex);
}

// =============================================================================
// Demo 5: Morph Queue
// =============================================================================

void demo_5_morph_queue() {
    print_demo_header("Demo 5: Deferred Morph Operations with Queue");
    
    // Create complex
    SimplicialComplex complex;
    auto v0 = complex.add_vertex();
    auto v1 = complex.add_vertex();
    auto v2 = complex.add_vertex();
    auto t = complex.add_triangle(v0, v1, v2);
    
    std::cout << "\nInitial state:" << std::endl;
    print_complex_stats(complex);
    
    // Create absurdity field
    AbsurdityField field;
    field.set(t, FuzzyInterval(0.8, 0.9, 0.95));
    
    // Create morph rule
    MorphRule rule(
        "delayed_split",
        MorphType::SPLIT,
        TriggerCondition::ABSURDITY_THRESHOLD,
        FuzzyInterval(0.7, 0.8, 0.9)
    );
    rule.probability = 1.0;
    
    // Create queue and enqueue morphs
    MorphQueue queue;
    queue.enqueue(QueuedMorph(rule, t, 0.5));  // Split after 0.5s
    queue.enqueue(QueuedMorph(rule, t, 1.0));  // Split again after 1.0s
    
    std::cout << "\nEnqueued " << queue.size() << " morph operations" << std::endl;
    
    // Simulate time progression
    MorphContext context(complex, field, 0.0, 0.1);
    
    std::cout << "\nTime 0.3: No operations ready" << std::endl;
    auto results0 = queue.execute_ready(0.3, context);
    std::cout << "Executed: " << results0.size() << " operations" << std::endl;
    
    std::cout << "\nTime 0.6: First operation ready" << std::endl;
    context.time = 0.6;
    auto results1 = queue.execute_ready(0.6, context);
    std::cout << "Executed: " << results1.size() << " operations" << std::endl;
    if (!results1.empty()) {
        print_morph_result(results1[0]);
        print_complex_stats(complex);
    }
    
    std::cout << "\nTime 1.1: Second operation ready" << std::endl;
    context.time = 1.1;
    auto results2 = queue.execute_ready(1.1, context);
    std::cout << "Executed: " << results2.size() << " operations" << std::endl;
    if (!results2.empty()) {
        print_morph_result(results2[0]);
        print_complex_stats(complex);
    }
    
    std::cout << "\nQueue size: " << queue.size() << std::endl;
}

// =============================================================================
// Demo 6: Statistics Tracking
// =============================================================================

void demo_6_statistics() {
    print_demo_header("Demo 6: Morph Statistics");
    
    // Create complex with multiple simplices
    SimplicialComplex complex;
    std::vector<VertexID> vertices;
    for (int i = 0; i < 6; ++i) {
        vertices.push_back(complex.add_vertex());
    }
    
    auto t1 = complex.add_triangle(vertices[0], vertices[1], vertices[2]);
    auto t2 = complex.add_triangle(vertices[2], vertices[3], vertices[4]);
    auto t3 = complex.add_triangle(vertices[1], vertices[4], vertices[5]);
    
    std::cout << "\nInitial state:" << std::endl;
    print_complex_stats(complex);
    
    // Create absurdity field
    AbsurdityField field;
    field.set(t1, FuzzyInterval(0.85, 0.95, 0.98));  // High -> split
    field.set(t2, FuzzyInterval(0.12, 0.18, 0.90));  // Low -> merge
    field.set(t3, FuzzyInterval(0.65, 0.75, 0.88));  // Medium -> expand
    
    // Add rules
    TopologyMorph morph;
    auto rules = MorphPresets::default_rules();
    for (const auto& rule : rules) {
        morph.add_rule(rule);
    }
    
    MorphContext context(complex, field, 0.0, 0.1);
    
    // Apply rules
    auto results = morph.apply_rules(context);
    
    std::cout << "\nApplied " << results.size() << " morph operations" << std::endl;
    for (size_t i = 0; i < results.size(); ++i) {
        std::cout << "\nOperation " << (i + 1) << ":" << std::endl;
        std::cout << "  Success: " << (results[i].success ? "Yes" : "No") << std::endl;
        std::cout << "  Message: " << results[i].message << std::endl;
    }
    
    // Show statistics
    std::cout << "\nFinal state:" << std::endl;
    print_complex_stats(complex);
    
    const auto& stats = morph.get_stats();
    std::cout << "\nMorph Statistics:" << std::endl;
    std::cout << "  Total operations: " << stats.total_operations << std::endl;
    std::cout << "  Successful: " << stats.successful_operations << std::endl;
    std::cout << "  Failed: " << stats.failed_operations << std::endl;
    std::cout << "  Total absurdity change: " << stats.total_absurdity_change << std::endl;
    
    std::cout << "\nOperation counts:" << std::endl;
    std::cout << "  SPLIT: " << stats.operation_counts.at(MorphType::SPLIT) << std::endl;
    std::cout << "  MERGE: " << stats.operation_counts.at(MorphType::MERGE) << std::endl;
    std::cout << "  DELETE: " << stats.operation_counts.at(MorphType::DELETE) << std::endl;
    std::cout << "  EXPAND: " << stats.operation_counts.at(MorphType::EXPAND) << std::endl;
}

// =============================================================================
// Demo 7: Complex Evolution
// =============================================================================

void demo_7_evolution() {
    print_demo_header("Demo 7: Complex Evolution Over Time");
    
    // Create initial complex
    SimplicialComplex complex;
    std::vector<VertexID> vertices;
    for (int i = 0; i < 8; ++i) {
        vertices.push_back(complex.add_vertex());
    }
    
    auto t1 = complex.add_triangle(vertices[0], vertices[1], vertices[2]);
    auto t2 = complex.add_triangle(vertices[1], vertices[3], vertices[2]);
    auto t3 = complex.add_triangle(vertices[2], vertices[4], vertices[5]);
    auto t4 = complex.add_triangle(vertices[3], vertices[5], vertices[4]);
    auto t5 = complex.add_triangle(vertices[5], vertices[6], vertices[7]);
    
    std::cout << "\nInitial state (time 0.0):" << std::endl;
    print_complex_stats(complex);
    
    // Create absurdity field
    AbsurdityField field;
    field.set(t1, FuzzyInterval(0.4, 0.5, 0.8));
    field.set(t2, FuzzyInterval(0.5, 0.6, 0.8));
    field.set(t3, FuzzyInterval(0.6, 0.7, 0.85));
    field.set(t4, FuzzyInterval(0.7, 0.8, 0.88));
    field.set(t5, FuzzyInterval(0.8, 0.9, 0.92));
    
    // Setup evolution
    EvolutionParams params;
    StochasticEvolution evolution(params);
    
    TopologyMorph morph;
    auto rules = MorphPresets::default_rules();
    for (const auto& rule : rules) {
        morph.add_rule(rule);
    }
    
    // Evolve over multiple time steps
    for (int step = 1; step <= 5; ++step) {
        std::cout << "\n--- Step " << step << " (time " << step * 0.2 << ") ---" << std::endl;
        
        // Evolve absurdity
        for (auto [id, simplex] : complex.get_simplices()) {
            auto current = field.get(id);
            auto next = evolution.step(current, 0.2);
            field.set(id, next);
            
            std::cout << "  Simplex " << id << " absurdity: " 
                      << current.to_string() << " -> " 
                      << next.to_string() << std::endl;
        }
        
        // Apply morph rules
        MorphContext context(complex, field, step * 0.2, 0.2);
        auto results = morph.apply_rules(context);
        
        std::cout << "  Applied " << results.size() << " morph operations" << std::endl;
        for (const auto& result : results) {
            if (result.success) {
                std::cout << "    - " << result.message << std::endl;
            }
        }
        
        print_complex_stats(complex);
    }
    
    // Final statistics
    const auto& stats = morph.get_stats();
    std::cout << "\nFinal Statistics:" << std::endl;
    std::cout << "  Total operations: " << stats.total_operations << std::endl;
    std::cout << "  Successful: " << stats.successful_operations << std::endl;
}

// =============================================================================
// Main
// =============================================================================

int main() {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "  Phase 9b: Dynamic Topology Morph Demos" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    demo_1_basic_split();
    demo_2_low_absurdity_merge();
    demo_3_expand_contract();
    demo_4_presets();
    demo_5_morph_queue();
    demo_6_statistics();
    demo_7_evolution();
    
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "  All demos completed!" << std::endl;
    std::cout << std::string(60, '=') << "\n" << std::endl;
    
    return 0;
}
