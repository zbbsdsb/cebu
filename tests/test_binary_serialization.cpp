#include "cebu/serialization.h"
#include "cebu/simplicial_complex_narrative.h"
#include <iostream>
#include <cassert>
#include <fstream>

using namespace cebu;

void test_basic_binary_serialize() {
    std::cout << "Testing basic binary serialization..." << std::endl;
    
    SimplicialComplex complex;
    
    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    
    complex.add_edge(v0, v1);
    complex.add_edge(v1, v2);
    complex.add_triangle(v0, v1, v2);
    
    // Serialize
    auto data = BinarySerializer::serialize(complex);
    
    std::cout << "Serialized size: " << data.size() << " bytes" << std::endl;
    assert(data.size() > 0);
    
    // Deserialize
    SimplicialComplex restored = BinarySerializer::deserialize(data);
    
    assert(restored.vertex_count() == 3);
    assert(restored.get_simplices_of_dimension(1).size() == 2);
    assert(restored.get_simplices_of_dimension(2).size() == 1);
    
    std::cout << "✓ Basic binary serialization passed" << std::endl;
}

void test_labeled_binary_serialize() {
    std::cout << "Testing labeled binary serialization..." << std::endl;
    
    SimplicialComplexLabeled<double> complex;
    
    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    SimplexID edge = complex.add_edge(v0, v1);
    
    complex.set_label(edge, 0.8);
    
    // Serialize
    auto data = BinarySerializer::serialize_labeled(complex);
    
    std::cout << "Serialized size: " << data.size() << " bytes" << std::endl;
    
    // Deserialize
    auto restored = BinarySerializer::deserialize_labeled<double>(data);
    
    assert(restored.vertex_count() == 2);
    assert(restored.get_simplices_of_dimension(1).size() == 1);
    
    auto label_opt = restored.get_label(edge);
    assert(label_opt.has_value());
    assert(std::abs(*label_opt - 0.8) < 0.0001);
    
    std::cout << "✓ Labeled binary serialization passed" << std::endl;
}

void test_absurdity_binary_serialize() {
    std::cout << "Testing Absurdity binary serialization..." << std::endl;
    
    SimplicialComplexLabeled<Absurdity> complex;
    
    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    SimplexID edge = complex.add_edge(v0, v1);
    
    Absurdity label(0.3, 0.7, 0.9);
    complex.set_label(edge, label);
    
    // Serialize
    auto data = BinarySerializer::serialize_labeled(complex);
    
    std::cout << "Serialized size: " << data.size() << " bytes" << std::endl;
    
    // Deserialize
    auto restored = BinarySerializer::deserialize_labeled<Absurdity>(data);
    
    auto label_opt = restored.get_label(edge);
    assert(label_opt.has_value());
    assert(std::abs(label_opt->lower() - 0.3) < 0.0001);
    assert(std::abs(label_opt->upper() - 0.7) < 0.0001);
    assert(std::abs(label_opt->confidence() - 0.9) < 0.0001);
    
    std::cout << "✓ Absurdity binary serialization passed" << std::endl;
}

void test_narrative_binary_serialize() {
    std::cout << "Testing narrative binary serialization..." << std::endl;
    
    SimplicialComplexNarrative<double> complex(0.0, 10.0);
    
    // Create topology
    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    SimplexID edge0 = complex.add_edge(v0, v1);
    SimplexID edge1 = complex.add_edge(v1, v2);
    SimplexID tri = complex.add_triangle(v0, v1, v2);
    
    // Set labels
    complex.set_label(edge0, 0.5);
    complex.set_label(edge1, 0.7);
    complex.set_label(tri, 0.9);
    
    // Add timeline milestones
    complex.timeline().add_milestone(2.5, "First event");
    complex.timeline().add_milestone(5.0, "Climax");
    complex.timeline().add_milestone(7.5, "Resolution");
    
    // Add events
    AbsurdityContext impact1;
    impact1.surprisal = 0.8;
    impact1.logical_deviation = 0.6;
    impact1.user_laughter = 0.5;
    impact1.narrative_tension = 0.7;
    impact1.dt = 0.1;
    
    complex.add_event("Twist", 2.5, {edge0, edge1}, impact1);
    
    // Set current time
    complex.evolve_to(5.0);
    
    // Serialize
    auto data = BinarySerializer::serialize_narrative(complex);
    
    std::cout << "Serialized size: " << data.size() << " bytes" << std::endl;
    std::cout << "  - Vertices: " << complex.vertex_count() << std::endl;
    std::cout << "  - Edges: " << complex.get_simplices_of_dimension(1).size() << std::endl;
    std::cout << "  - Triangles: " << complex.get_simplices_of_dimension(2).size() << std::endl;
    std::cout << "  - Milestones: " << complex.timeline().get_milestones().size() << std::endl;
    std::cout << "  - Events: " << complex.events().get_all_events().size() << std::endl;
    
    // Deserialize
    auto restored = BinarySerializer::deserialize_narrative<double>(data);
    
    // Verify topology
    assert(restored.vertex_count() == 3);
    assert(restored.get_simplices_of_dimension(1).size() == 2);
    assert(restored.get_simplices_of_dimension(2).size() == 1);
    
    // Verify labels
    auto label0 = restored.get_label(edge0);
    assert(label0.has_value());
    assert(std::abs(*label0 - 0.5) < 0.0001);
    
    auto label1 = restored.get_label(edge1);
    assert(label1.has_value());
    assert(std::abs(*label1 - 0.7) < 0.0001);
    
    auto label_tri = restored.get_label(tri);
    assert(label_tri.has_value());
    assert(std::abs(*label_tri - 0.9) < 0.0001);
    
    // Verify timeline milestones
    auto milestones = restored.timeline().get_milestones();
    assert(milestones.size() == 3);
    auto it = milestones.find(2.5);
    assert(it != milestones.end());
    assert(it->second == "First event");
    
    it = milestones.find(5.0);
    assert(it != milestones.end());
    assert(it->second == "Climax");
    
    it = milestones.find(7.5);
    assert(it != milestones.end());
    assert(it->second == "Resolution");
    
    // Verify events
    auto events = restored.events().get_all_events();
    assert(events.size() == 1);
    
    const StoryEvent& event = events.begin()->second;
    assert(event.description == "Twist");
    assert(std::abs(event.timestamp - 2.5) < 0.0001);
    assert(event.affected_simplices.size() == 2);
    assert(std::abs(event.impact.surprisal - 0.8) < 0.0001);
    assert(std::abs(event.impact.logical_deviation - 0.6) < 0.0001);
    assert(std::abs(event.impact.user_laughter - 0.5) < 0.0001);
    assert(std::abs(event.impact.narrative_tension - 0.7) < 0.0001);
    assert(std::abs(event.impact.dt - 0.1) < 0.0001);
    
    std::cout << "✓ Narrative binary serialization passed" << std::endl;
}

void test_save_to_file() {
    std::cout << "Testing save to file..." << std::endl;
    
    SimplicialComplexNarrative<double> complex(0.0, 10.0);
    
    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    SimplexID edge = complex.add_edge(v0, v1);
    complex.set_label(edge, 0.75);
    
    complex.timeline().add_milestone(5.0, "Midpoint");
    
    AbsurdityContext impact;
    impact.surprisal = 0.9;
    complex.add_event("Event", 5.0, {edge}, impact);
    
    // Serialize
    auto data = BinarySerializer::serialize_narrative(complex);
    
    // Write to file
    std::ofstream out("test_narrative.bin", std::ios::binary);
    out.write(reinterpret_cast<const char*>(data.data()), data.size());
    out.close();
    
    std::cout << "Saved to test_narrative.bin (" << data.size() << " bytes)" << std::endl;
    
    // Read back
    std::ifstream in("test_narrative.bin", std::ios::binary);
    in.seekg(0, std::ios::end);
    size_t size = in.tellg();
    in.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> read_data(size);
    in.read(reinterpret_cast<char*>(read_data.data()), size);
    in.close();
    
    // Deserialize
    auto restored = BinarySerializer::deserialize_narrative<double>(read_data);
    
    assert(restored.vertex_count() == 2);
    auto label = restored.get_label(edge);
    assert(label.has_value());
    assert(std::abs(*label - 0.75) < 0.0001);
    
    std::cout << "✓ Save to file passed" << std::endl;
}

void test_empty_complex() {
    std::cout << "Testing empty complex serialization..." << std::endl;
    
    SimplicialComplex complex;
    auto data = BinarySerializer::serialize(complex);
    auto restored = BinarySerializer::deserialize(data);
    
    assert(restored.vertex_count() == 0);
    assert(restored.simplex_count() == 0);
    
    std::cout << "✓ Empty complex serialization passed" << std::endl;
}

void test_large_complex() {
    std::cout << "Testing large complex serialization..." << std::endl;
    
    SimplicialComplexNarrative<double> complex(0.0, 100.0);
    
    // Create a grid of vertices
    const int grid_size = 10;
    std::vector<std::vector<VertexID>> grid(grid_size, std::vector<VertexID>(grid_size));
    
    for (int i = 0; i < grid_size; ++i) {
        for (int j = 0; j < grid_size; ++j) {
            grid[i][j] = complex.add_vertex();
        }
    }
    
    // Create horizontal edges
    for (int i = 0; i < grid_size; ++i) {
        for (int j = 0; j < grid_size - 1; ++j) {
            complex.add_edge(grid[i][j], grid[i][j+1]);
        }
    }
    
    // Create vertical edges
    for (int i = 0; i < grid_size - 1; ++i) {
        for (int j = 0; j < grid_size; ++j) {
            complex.add_edge(grid[i][j], grid[i+1][j]);
        }
    }
    
    size_t vertex_count = complex.vertex_count();
    size_t edge_count = complex.get_simplices_of_dimension(1).size();
    
    std::cout << "  Vertices: " << vertex_count << std::endl;
    std::cout << "  Edges: " << edge_count << std::endl;
    
    // Serialize
    auto data = BinarySerializer::serialize_narrative(complex);
    
    std::cout << "Serialized size: " << data.size() << " bytes" << std::endl;
    std::cout << "Bytes per vertex: " << (data.size() / vertex_count) << std::endl;
    
    // Deserialize
    auto restored = BinarySerializer::deserialize_narrative<double>(data);
    
    assert(restored.vertex_count() == vertex_count);
    assert(restored.get_simplices_of_dimension(1).size() == edge_count);
    
    std::cout << "✓ Large complex serialization passed" << std::endl;
}

int main() {
    std::cout << "=== Cebu Binary Serialization Tests ===" << std::endl << std::endl;
    
    try {
        test_empty_complex();
        test_basic_binary_serialize();
        test_labeled_binary_serialize();
        test_absurdity_binary_serialize();
        test_narrative_binary_serialize();
        test_save_to_file();
        test_large_complex();
        
        std::cout << std::endl;
        std::cout << "=====================================" << std::endl;
        std::cout << "All binary serialization tests passed! ✅" << std::endl;
        std::cout << "=====================================" << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception" << std::endl;
        return 1;
    }
}
