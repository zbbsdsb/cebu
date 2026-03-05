#include "cebu/simplicial_complex.h"
#include "cebu/simplicial_complex_labeled.h"
#include "cebu/simplicial_complex_narrative.h"
#include "cebu/json_serialization.h"
#include <iostream>
#include <cassert>
#include <fstream>

using namespace cebu;

void test_basic_complex_json() {
    std::cout << "Testing basic complex JSON serialization...\n";
    
    SimplicialComplex complex;
    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    complex.add_edge(v0, v1);
    complex.add_edge(v1, v2);
    complex.add_edge(v2, v0);
    complex.add_triangle(v0, v1, v2);
    
    // Serialize to JSON
    nlohmann::json j = JsonSerializer::serialize(complex);
    
    // Check JSON structure
    assert(j["format"] == "cebu");
    assert(j.contains("version"));
    assert(j.contains("simplices"));
    assert(j["simplices"].is_array());
    
    std::cout << "  JSON structure validated\n";
    
    // Pretty print
    std::string pretty = JsonSerializer::pretty_print(j, 2);
    std::cout << "  Pretty printed JSON:\n" << pretty.substr(0, 500) << "...\n";
    
    // Validate
    bool valid = JsonSerializer::validate(j);
    assert(valid);
    std::cout << "  JSON validation passed\n";
    
    // Save to file
    std::ofstream out("test_basic_complex.json");
    out << pretty;
    out.close();
    std::cout << "  Saved to test_basic_complex.json\n";
    
    // Load from file
    std::ifstream in("test_basic_complex.json");
    nlohmann::json j_loaded;
    in >> j_loaded;
    in.close();
    
    assert(JsonSerializer::validate(j_loaded));
    std::cout << "  ✓ Basic complex JSON serialization passed\n\n";
}

void test_labeled_complex_json() {
    std::cout << "Testing labeled complex JSON serialization...\n";
    
    SimplicialComplexLabeled<double> complex;
    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    complex.add_edge(v0, v1);
    complex.add_edge(v1, v2);
    complex.add_edge(v2, v0);
    complex.add_triangle(v0, v1, v2);
    
    // Add labels
    complex.set_label(v0, 0.5);
    complex.set_label(v1, 0.8);
    complex.set_label(v2, 0.3);
    
    // Serialize to JSON
    nlohmann::json j = JsonSerializer::serialize_labeled(complex);
    
    // Check labels
    assert(j.contains("labels"));
    assert(j["labels"].is_object());
    assert(j["labels"].contains(std::to_string(v0)));
    assert(j["labels"][std::to_string(v0)] == 0.5);
    
    std::cout << "  Labels validated\n";
    
    // Pretty print
    std::string pretty = JsonSerializer::pretty_print(j, 2);
    std::ofstream out("test_labeled_complex.json");
    out << pretty;
    out.close();
    std::cout << "  Saved to test_labeled_complex.json\n";
    
    std::cout << "  ✓ Labeled complex JSON serialization passed\n\n";
}

void test_narrative_complex_json() {
    std::cout << "Testing narrative complex JSON serialization...\n";
    
    SimplicialComplexNarrative<double> complex(0.0, 100.0);
    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    complex.add_edge(v0, v1);
    
    complex.set_label(v0, 0.5);
    complex.set_label(v1, 0.8);
    
    // Serialize to JSON
    nlohmann::json j = JsonSerializer::serialize_narrative(complex);
    
    // Check timeline
    assert(j.contains("timeline"));
    assert(j["timeline"]["min_time"] == 0.0);
    assert(j["timeline"]["max_time"] == 100.0);
    assert(j["timeline"]["current_time"] == 0.0);
    
    std::cout << "  Timeline validated\n";
    
    // Check events
    assert(j.contains("events"));
    assert(j["events"].is_array());
    
    std::cout << "  Events validated\n";
    
    // Pretty print
    std::string pretty = JsonSerializer::pretty_print(j, 2);
    std::ofstream out("test_narrative_complex.json");
    out << pretty;
    out.close();
    std::cout << "  Saved to test_narrative_complex.json\n";
    
    std::cout << "  ✓ Narrative complex JSON serialization passed\n\n";
}

void test_json_schema() {
    std::cout << "Testing JSON schema...\n";
    
    // Get schema
    nlohmann::json schema = JsonSerializer::get_schema();
    
    assert(schema.contains("$schema"));
    assert(schema.contains("type"));
    assert(schema["type"] == "object");
    
    std::cout << "  Schema structure validated\n";
    
    // Save schema
    std::ofstream out("cebu_schema.json");
    out << schema.dump(2);
    out.close();
    std::cout << "  Saved schema to cebu_schema.json\n";
    
    std::cout << "  ✓ JSON schema test passed\n\n";
}

void test_large_complex_json() {
    std::cout << "Testing large complex JSON serialization...\n";
    
    SimplicialComplex complex;
    
    // Create 10x10 grid of vertices
    const int grid_size = 10;
    std::vector<std::vector<VertexID>> vertices(grid_size, 
        std::vector<VertexID>(grid_size));
    
    for (int i = 0; i < grid_size; ++i) {
        for (int j = 0; j < grid_size; ++j) {
            vertices[i][j] = complex.add_vertex();
        }
    }
    
    // Create edges
    int edge_count = 0;
    for (int i = 0; i < grid_size; ++i) {
        for (int j = 0; j < grid_size; ++j) {
            if (i < grid_size - 1) {
                complex.add_edge(vertices[i][j], vertices[i+1][j]);
                edge_count++;
            }
            if (j < grid_size - 1) {
                complex.add_edge(vertices[i][j], vertices[i][j+1]);
                edge_count++;
            }
        }
    }
    
    std::cout << "  Created complex with " << complex.vertex_count() 
              << " vertices and " << edge_count << " edges\n";
    
    // Serialize to JSON
    nlohmann::json j = JsonSerializer::serialize(complex);
    
    // Check statistics
    assert(j["statistics"]["vertex_count"] == complex.vertex_count());
    
    // Pretty print and save
    std::string pretty = JsonSerializer::pretty_print(j, 2);
    std::ofstream out("test_large_complex.json");
    out << pretty;
    out.close();
    
    std::cout << "  JSON size: " << pretty.size() << " bytes\n";
    std::cout << "  Saved to test_large_complex.json\n";
    std::cout << "  ✓ Large complex JSON serialization passed\n\n";
}

int main() {
    std::cout << "\n=== JSON Serialization Tests ===\n\n";
    
    try {
        test_basic_complex_json();
        test_labeled_complex_json();
        test_narrative_complex_json();
        test_json_schema();
        test_large_complex_json();
        
        std::cout << "=== All JSON Serialization Tests Passed! ===\n\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
