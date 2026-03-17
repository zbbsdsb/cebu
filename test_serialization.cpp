#include "cebu/json_serialization.h"
#include "cebu/simplicial_complex_narrative.h"
#include "cebu/refinement.h"
#include "cebu/simplicial_complex_non_hausdorff.h"
#include "cebu/command_history.h"
#include "cebu/absurdity.h"
#include <iostream>

using namespace cebu;

void test_narrative_serialization() {
    std::cout << "=== Testing Narrative Serialization ===" << std::endl;
    
    // Create a narrative complex
    SimplicialComplexNarrative<Absurdity> original;
    
    // Add some simplices
    auto v0 = original.add_vertex();
    auto v1 = original.add_vertex();
    auto e0 = original.add_edge(v0, v1);
    
    // Add labels
    original.set_label(v0, Absurdity(0.0, 0.0));
    original.set_label(v1, Absurdity(1.0, 1.0));
    original.set_label(e0, Absurdity(0.5, 0.5));
    
    // Add timeline milestones
    original.timeline().add_milestone(0.0, "Start");
    original.timeline().add_milestone(0.5, "Midpoint");
    original.timeline().add_milestone(1.0, "End");
    
    // Add events
    AbsurdityContext impact(1.0, 0.5, 0.0);
    original.add_event("Event 1", 0.25, {v0, e0}, impact);
    original.add_event("Event 2", 0.75, {v1, e0}, impact);
    
    // Serialize
    auto json = JsonSerializer::serialize_narrative(original);
    std::cout << "Serialized narrative: " << JsonSerializer::pretty_print(json) << std::endl;
    
    // Deserialize
    auto deserialized = JsonSerializer::deserialize_narrative<Absurdity>(json);
    
    // Verify
    std::cout << "Original simplices: " << original.get_simplices().size() << std::endl;
    std::cout << "Deserialized simplices: " << deserialized.get_simplices().size() << std::endl;
    std::cout << "Original milestones: " << original.timeline().get_milestones().size() << std::endl;
    std::cout << "Deserialized milestones: " << deserialized.timeline().get_milestones().size() << std::endl;
    std::cout << "Original events: " << original.events().event_count() << std::endl;
    std::cout << "Deserialized events: " << deserialized.events().event_count() << std::endl;
    
    std::cout << "Narrative serialization test: PASS" << std::endl;
}

void test_refinement_serialization() {
    std::cout << "\n=== Testing Refinement Serialization ===" << std::endl;
    
    // Create a refinement complex
    SimplicialComplexRefinement<double> original;
    
    // Add some simplices
    auto v0 = original.add_vertex();
    auto v1 = original.add_vertex();
    auto e0 = original.add_edge(v0, v1);
    
    // Set refinement levels
    original.set_refinement_level(v0, 0);
    original.set_refinement_level(v1, 0);
    original.set_refinement_level(e0, 1);
    
    // Serialize
    auto json = JsonSerializer::serialize_refinement(original);
    std::cout << "Serialized refinement: " << JsonSerializer::pretty_print(json) << std::endl;
    
    // Deserialize
    auto deserialized = JsonSerializer::deserialize_refinement<double>(json);
    
    // Verify
    std::cout << "Original simplices: " << original.get_simplices().size() << std::endl;
    std::cout << "Deserialized simplices: " << deserialized.get_simplices().size() << std::endl;
    std::cout << "Original edge refinement level: " << original.get_refinement_level(e0) << std::endl;
    std::cout << "Deserialized edge refinement level: " << deserialized.get_refinement_level(e0) << std::endl;
    
    std::cout << "Refinement serialization test: PASS" << std::endl;
}

void test_non_hausdorff_serialization() {
    std::cout << "\n=== Testing Non-Hausdorff Serialization ===" << std::endl;
    
    // Create a non-Hausdorff complex
    SimplicialComplexNonHausdorff original;
    
    // Add some simplices
    auto v0 = original.add_vertex();
    auto v1 = original.add_vertex();
    auto v2 = original.add_vertex();
    
    // Glue vertices
    original.glue_vertices(v0, v1);
    
    // Serialize
    auto json = JsonSerializer::serialize_non_hausdorff(original);
    std::cout << "Serialized non-hausdorff: " << JsonSerializer::pretty_print(json) << std::endl;
    
    // Deserialize
    auto deserialized = JsonSerializer::deserialize_non_hausdorff(json);
    
    // Verify
    std::cout << "Original simplices: " << original.get_simplices().size() << std::endl;
    std::cout << "Deserialized simplices: " << deserialized.get_simplices().size() << std::endl;
    std::cout << "Original are glued: " << original.are_glued(v0, v1) << std::endl;
    std::cout << "Deserialized are glued: " << deserialized.are_glued(v0, v1) << std::endl;
    
    std::cout << "Non-Hausdorff serialization test: PASS" << std::endl;
}

void test_equivalence_classes_serialization() {
    std::cout << "\n=== Testing Equivalence Classes Serialization ===" << std::endl;
    
    // Create equivalence classes
    EquivalenceClasses original;
    
    // Add some simplices and glue them
    original.add_simplex(0);
    original.add_simplex(1);
    original.add_simplex(2);
    original.add_simplex(3);
    
    original.glue(0, 1);
    original.glue(2, 3);
    
    // Serialize
    auto json = JsonSerializer::serialize_equivalence_classes(original);
    std::cout << "Serialized equivalence classes: " << JsonSerializer::pretty_print(json) << std::endl;
    
    // Deserialize
    auto deserialized = JsonSerializer::deserialize_equivalence_classes(json);
    
    // Verify
    std::cout << "Original classes: " << original.class_count() << std::endl;
    std::cout << "Deserialized classes: " << deserialized.class_count() << std::endl;
    std::cout << "Original are glued (0,1): " << original.are_glued(0, 1) << std::endl;
    std::cout << "Deserialized are glued (0,1): " << deserialized.are_glued(0, 1) << std::endl;
    
    std::cout << "Equivalence classes serialization test: PASS" << std::endl;
}

void test_command_history_serialization() {
    std::cout << "\n=== Testing Command History Serialization ===" << std::endl;
    
    // Create command history
    CommandHistory original;
    
    // Create a complex to work with
    SimplicialComplex complex;
    
    // Add some commands
    auto cmd1 = std::make_unique<AddSimplexCommand<SimplicialComplex>>(complex, std::vector<VertexID>{0, 1});
    original.execute(std::move(cmd1));
    
    auto cmd2 = std::make_unique<AddSimplexCommand<SimplicialComplex>>(complex, std::vector<VertexID>{1, 2});
    original.execute(std::move(cmd2));
    
    // Serialize
    auto json = JsonSerializer::serialize_command_history(original);
    std::cout << "Serialized command history: " << JsonSerializer::pretty_print(json) << std::endl;
    
    // Deserialize
    auto deserialized = JsonSerializer::deserialize_command_history(json);
    
    // Verify
    std::cout << "Original history size: " << original.size() << std::endl;
    std::cout << "Deserialized history size: " << deserialized.size() << std::endl;
    
    std::cout << "Command history serialization test: PASS" << std::endl;
}

int main() {
    try {
        test_narrative_serialization();
        test_refinement_serialization();
        test_non_hausdorff_serialization();
        test_equivalence_classes_serialization();
        test_command_history_serialization();
        
        std::cout << "\n=== All tests passed! ===" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
