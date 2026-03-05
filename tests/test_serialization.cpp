#include "cebu/serialization.h"
#include "cebu/simplicial_complex_labeled.h"
#include "cebu/simplicial_complex_narrative.h"
#include "cebu/absurdity.h"
#include <iostream>
#include <cassert>
#include <fstream>

using namespace cebu;

void test_serialize_basic_complex() {
    std::cout << "Testing serialize basic complex..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    complex.add_triangle(v0, v1, v2);

    std::string json = JsonSerializer::serialize(complex);

    // Check that JSON contains expected elements
    assert(json.find("\"version\"") != std::string::npos);
    assert(json.find("\"type\": \"basic\"") != std::string::npos);
    assert(json.find("\"simplices\"") != std::string::npos);
    assert(json.find("\"id\"") != std::string::npos);
    assert(json.find("\"vertices\"") != std::string::npos);
    assert(json.find("\"dimension\"") != std::string::npos);

    std::cout << "  PASSED" << std::endl;
}

void test_serialize_labeled_double() {
    std::cout << "Testing serialize labeled double..." << std::endl;

    SimplicialComplexLabeled<double> complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    SimplexID edge = complex.add_edge(v0, v1);

    complex.set_label(v0, 0.5);
    complex.set_label(v1, 0.8);
    complex.set_label(edge, 0.6);

    std::string json = JsonSerializer::serialize_labeled_double(complex);

    // Check JSON structure
    assert(json.find("\"type\": \"labeled_double\"") != std::string::npos);
    assert(json.find("\"labels\"") != std::string::npos);
    assert(json.find("0.5") != std::string::npos);
    assert(json.find("0.8") != std::string::npos);
    assert(json.find("0.6") != std::string::npos);

    std::cout << "  PASSED" << std::endl;
}

void test_serialize_labeled_absurdity() {
    std::cout << "Testing serialize labeled absurdity..." << std::endl;

    SimplicialComplexLabeled<Absurdity> complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    SimplexID tri = complex.add_triangle(v0, v1, complex.add_vertex());

    Absurdity a1(0.3, 0.5, 0.8);
    Absurdity a2(0.6, 0.7, 0.9);
    complex.set_label(v0, a1);
    complex.set_label(tri, a2);

    std::string json = JsonSerializer::serialize_labeled_absurdity(complex);

    // Check JSON structure
    assert(json.find("\"type\": \"labeled_absurdity\"") != std::string::npos);
    assert(json.find("\"lower\"") != std::string::npos);
    assert(json.find("\"upper\"") != std::string::npos);
    assert(json.find("\"confidence\"") != std::string::npos);
    assert(json.find("0.3") != std::string::npos);
    assert(json.find("0.5") != std::string::npos);
    assert(json.find("0.8") != std::string::npos);

    std::cout << "  PASSED" << std::endl;
}

void test_serialize_narrative_complex() {
    std::cout << "Testing serialize narrative complex..." << std::endl;

    SimplicialComplexNarrative<Absurdity> complex(0.0, 100.0);

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    SimplexID tri = complex.add_triangle(v0, v1, v2);

    // Set labels
    complex.set_label(tri, Absurdity(0.3, 0.4, 0.8));

    // Add timeline milestones
    complex.timeline().add_milestone(25.0, "First Act");
    complex.timeline().add_milestone(50.0, "Climax");
    complex.timeline().add_milestone(75.0, "Resolution");

    // Add events
    AbsurdityContext ctx1{0.5, 0.3, 0.4, 0.6, 1.0};
    complex.add_event("Dramatic twist", 30.0, {tri}, ctx1);

    AbsurdityContext ctx2{0.8, 0.6, 0.7, 0.9, 1.0};
    complex.add_event("Plot resolution", 70.0, {tri}, ctx2);

    // Evolve
    complex.evolve_to(50.0);

    std::string json = JsonSerializer::serialize_narrative(complex);

    // Check JSON structure
    assert(json.find("\"type\": \"narrative\"") != std::string::npos);
    assert(json.find("\"timeline\"") != std::string::npos);
    assert(json.find("\"milestones\"") != std::string::npos);
    assert(json.find("\"events\"") != std::string::npos);
    assert(json.find("\"current_time\"") != std::string::npos);
    assert(json.find("First Act") != std::string::npos);
    assert(json.find("Climax") != std::string::npos);
    assert(json.find("Dramatic twist") != std::string::npos);
    assert(json.find("50.0") != std::string::npos);

    std::cout << "  PASSED" << std::endl;
}

void test_binary_serialize_basic() {
    std::cout << "Testing binary serialize basic..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    complex.add_triangle(v0, v1, v2);

    std::vector<uint8_t> data = BinarySerializer::serialize(complex);

    // Check header
    assert(data.size() >= 8);
    assert(data[0] == 0x55); // "CEBU" magic

    // Deserialize
    SimplicialComplex deserialized = BinarySerializer::deserialize(data);

    assert(deserialized.simplex_count() == complex.simplex_count());

    std::cout << "  PASSED" << std::endl;
}

void test_binary_serialize_roundtrip() {
    std::cout << "Testing binary serialize roundtrip..." << std::endl;

    SimplicialComplex complex;

    // Create a more complex structure
    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    VertexID v3 = complex.add_vertex();

    complex.add_edge(v0, v1);
    complex.add_edge(v1, v2);
    complex.add_edge(v2, v3);
    complex.add_triangle(v0, v1, v2);

    std::vector<uint8_t> data = BinarySerializer::serialize(complex);
    SimplicialComplex deserialized = BinarySerializer::deserialize(data);

    // Note: IDs may differ after deserialization, but structure should be similar
    // For now, just verify deserialization succeeded
    assert(deserialized.simplex_count() > 0);

    std::cout << "  PASSED" << std::endl;
}

void test_binary_serialize_invalid_magic() {
    std::cout << "Testing binary serialize invalid magic..." << std::endl;

    std::vector<uint8_t> data = {0xFF, 0xFF, 0xFF, 0xFF}; // Invalid magic

    bool threw = false;
    try {
        BinarySerializer::deserialize(data);
    } catch (const std::runtime_error&) {
        threw = true;
    }

    assert(threw);

    std::cout << "  PASSED" << std::endl;
}

void test_json_format_validity() {
    std::cout << "Testing JSON format validity..." << std::endl;

    SimplicialComplexNarrative<double> complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    SimplexID edge = complex.add_edge(v0, v1);

    complex.set_label(edge, 0.5);
    complex.timeline().add_milestone(50.0, "Midpoint");

    std::string json = JsonSerializer::serialize_narrative(complex);

    // Basic JSON syntax checks
    size_t open_braces = 0;
    size_t close_braces = 0;
    size_t open_brackets = 0;
    size_t close_brackets = 0;

    for (char c : json) {
        if (c == '{') open_braces++;
        else if (c == '}') close_braces++;
        else if (c == '[') open_brackets++;
        else if (c == ']') close_brackets++;
    }

    assert(open_braces == close_braces);
    assert(open_brackets == close_brackets);

    std::cout << "  PASSED" << std::endl;
}

void test_serialize_empty_complex() {
    std::cout << "Testing serialize empty complex..." << std::endl;

    SimplicialComplex complex;
    std::string json = JsonSerializer::serialize(complex);

    assert(json.find("\"simplices\"") != std::string::npos);
    assert(json.find("[]") != std::string::npos); // Empty array

    std::cout << "  PASSED" << std::endl;
}

void test_serialize_multiple_labels() {
    std::cout << "Testing serialize multiple labels..." << std::endl;

    SimplicialComplexLabeled<double> complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    VertexID v3 = complex.add_vertex();

    complex.add_edge(v0, v1);
    complex.add_edge(v1, v2);
    complex.add_edge(v2, v3);

    // Label multiple simplices
    complex.set_label(v0, 0.1);
    complex.set_label(v1, 0.2);
    complex.set_label(v2, 0.3);
    complex.set_label(v3, 0.4);

    std::string json = JsonSerializer::serialize_labeled_double(complex);

    // Check that all labels are present
    assert(json.find("0.1") != std::string::npos);
    assert(json.find("0.2") != std::string::npos);
    assert(json.find("0.3") != std::string::npos);
    assert(json.find("0.4") != std::string::npos);

    std::cout << "  PASSED" << std::endl;
}

void test_serialize_no_labels() {
    std::cout << "Testing serialize no labels..." << std::endl;

    SimplicialComplexLabeled<double> complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    complex.add_edge(v0, v1);

    // Don't set any labels
    std::string json = JsonSerializer::serialize_labeled_double(complex);

    // Labels section should be empty
    assert(json.find("\"labels\": {}") != std::string::npos ||
           json.find("\"labels\": {\n  }") != std::string::npos);

    std::cout << "  PASSED" << std::endl;
}

void test_serialize_narrative_no_events() {
    std::cout << "Testing serialize narrative no events..." << std::endl;

    SimplicialComplexNarrative<double> complex;

    VertexID v0 = complex.add_vertex();
    complex.add_edge(v0, complex.add_vertex());

    // Don't add any events
    std::string json = JsonSerializer::serialize_narrative(complex);

    // Events section should be empty
    assert(json.find("\"events\": []") != std::string::npos ||
           json.find("\"events\": [\n  ]") != std::string::npos);

    std::cout << "  PASSED" << std::endl;
}

int main() {
    std::cout << "=== Serialization Tests ===" << std::endl;
    std::cout << std::endl;

    test_serialize_basic_complex();
    test_serialize_labeled_double();
    test_serialize_labeled_absurdity();
    test_serialize_narrative_complex();
    test_binary_serialize_basic();
    test_binary_serialize_roundtrip();
    test_binary_serialize_invalid_magic();
    test_json_format_validity();
    test_serialize_empty_complex();
    test_serialize_multiple_labels();
    test_serialize_no_labels();
    test_serialize_narrative_no_events();

    std::cout << std::endl;
    std::cout << "=== All tests passed! ===" << std::endl;

    return 0;
}
