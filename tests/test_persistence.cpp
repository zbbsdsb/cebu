#include "cebu/persistence.h"
#include "cebu/refinement.h"
#include "cebu/simplicial_complex_narrative.h"
#include "cebu/simplicial_complex_non_hausdorff.h"
#include <iostream>
#include <cassert>
#include <cmath>
#include <filesystem>

using namespace cebu;

void cleanup_test_files() {
    // Clean up test files
    std::filesystem::remove("test_complex.bin");
    std::filesystem::remove("test_complex.json");
    std::filesystem::remove("test_labeled.bin");
    std::filesystem::remove("test_narrative.bin");
    std::filesystem::remove("test_refinement.bin");
}

void test_basic_save_load() {
    std::cout << "Testing basic save/load..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();

    complex.add_edge(v0, v1);
    complex.add_edge(v1, v2);
    complex.add_triangle(v0, v1, v2);

    // Save to binary
    bool saved = Persistence::save(complex, "test_complex.bin");
    assert(saved);

    // Load back
    auto result = Persistence::load("test_complex.bin");
    assert(result.success);

    // Verify
    assert(result.complex.vertex_count() == 3);
    assert(result.complex.get_simplices_of_dimension(1).size() == 2);
    assert(result.complex.get_simplices_of_dimension(2).size() == 1);

    std::cout << "✓ Basic save/load passed" << std::endl;
}

void test_labeled_save_load() {
    std::cout << "Testing labeled save/load..." << std::endl;

    SimplicialComplexLabeled<double> complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    SimplexID edge = complex.add_edge(v0, v1);

    complex.set_label(v0, 0.5);
    complex.set_label(v1, 0.7);
    complex.set_label(edge, 0.9);

    // Save
    bool saved = Persistence::save_labeled(complex, "test_labeled.bin");
    assert(saved);

    // Load
    auto result = Persistence::load_labeled<double>("test_labeled.bin");
    assert(result.success);

    // Verify labels
    auto label0 = result.complex.get_label(v0);
    assert(label0.has_value());
    assert(std::abs(*label0 - 0.5) < 0.0001);

    auto label1 = result.complex.get_label(v1);
    assert(label1.has_value());
    assert(std::abs(*label1 - 0.7) < 0.0001);

    auto label_edge = result.complex.get_label(edge);
    assert(label_edge.has_value());
    assert(std::abs(*label_edge - 0.9) < 0.0001);

    std::cout << "✓ Labeled save/load passed" << std::endl;
}

void test_narrative_save_load() {
    std::cout << "Testing narrative save/load..." << std::endl;

    SimplicialComplexNarrative<double> complex(0.0, 10.0);

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    SimplexID edge = complex.add_edge(v0, v1);

    complex.set_label(edge, 0.75);
    complex.timeline().add_milestone(5.0, "Midpoint");

    AbsurdityContext impact;
    impact.surprisal = 0.8;
    impact.narrative_tension = 0.9;

    complex.add_event("Twist", 5.0, {edge}, impact);

    // Save
    bool saved = Persistence::save_narrative(complex, "test_narrative.bin");
    assert(saved);

    // Load
    auto result = Persistence::load_narrative<double>("test_narrative.bin");
    assert(result.success);

    // Verify
    assert(result.complex.vertex_count() == 2);

    auto milestones = result.complex.timeline().get_milestones();
    assert(milestones.size() == 1);
    auto it = milestones.find(5.0);
    assert(it != milestones.end());
    assert(it->second == "Midpoint");

    auto events = result.complex.events().get_all_events();
    assert(events.size() == 1);
    const StoryEvent& event = events.begin()->second;
    assert(event.description == "Twist");
    assert(std::abs(event.impact.surprisal - 0.8) < 0.0001);

    std::cout << "✓ Narrative save/load passed" << std::endl;
}

void test_refinement_save_load() {
    std::cout << "Testing refinement save/load..." << std::endl;

    SimplicialComplexRefinement<double> complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    SimplexID tri = complex.add_triangle(v0, v1, v2);

    complex.set_label(tri, 0.9);

    // Set refinement levels
    complex.set_refinement_level(tri, 2);

    // Save
    bool saved = Persistence::save_refinement(complex, "test_refinement.bin");
    assert(saved);

    // Load
    auto result = Persistence::load_refinement<double>("test_refinement.bin");
    assert(result.success);

    // Verify
    assert(result.complex.vertex_count() == 3);

    auto level = result.complex.get_refinement_level(tri);
    assert(level == 2);

    std::cout << "✓ Refinement save/load passed" << std::endl;
}

void test_format_detection() {
    std::cout << "Testing format detection..." << std::endl;

    assert(Persistence::detect_format("test.bin") == FileFormat::BINARY);
    assert(Persistence::detect_format("test.ceb") == FileFormat::BINARY);
    assert(Persistence::detect_format("test.json") == FileFormat::JSON);
    assert(Persistence::detect_format("test.BIN") == FileFormat::BINARY);
    assert(Persistence::detect_format("test.JSON") == FileFormat::JSON);
    assert(Persistence::detect_format("test") == FileFormat::BINARY); // Default

    std::cout << "✓ Format detection passed" << std::endl;
}

void test_get_metadata() {
    std::cout << "Testing get_metadata..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    complex.add_edge(v0, v1);

    Persistence::save(complex, "test_metadata.bin");

    auto metadata = Persistence::get_metadata("test_metadata.bin");

    assert(metadata.simplex_count > 0);
    assert(metadata.file_size > 0);
    assert(!metadata.cebu_version.empty());

    std::cout << "  Metadata - Simplex count: " << metadata.simplex_count << std::endl;
    std::cout << "  Metadata - File size: " << metadata.file_size << " bytes" << std::endl;
    std::cout << "  Metadata - Version: " << metadata.cebu_version << std::endl;

    std::filesystem::remove("test_metadata.bin");

    std::cout << "✓ Get metadata passed" << std::endl;
}

void test_validate_file() {
    std::cout << "Testing validate_file..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    complex.add_edge(v0, v0); // Self-loop for testing

    Persistence::save(complex, "test_validate.bin");

    bool valid = Persistence::validate_file("test_validate.bin");
    assert(valid);

    // Test invalid file
    {
        std::ofstream out("invalid.bin");
        out << "invalid data";
        out.close();
    }

    bool invalid = Persistence::validate_file("invalid.bin");
    assert(!invalid);

    std::filesystem::remove("test_validate.bin");
    std::filesystem::remove("invalid.bin");

    std::cout << "✓ Validate file passed" << std::endl;
}

void test_large_complex() {
    std::cout << "Testing large complex save/load..." << std::endl;

    SimplicialComplexLabeled<double> complex;

    // Create a 20x20 grid
    const int grid_size = 20;
    std::vector<std::vector<VertexID>> grid(grid_size, std::vector<VertexID>(grid_size));

    for (int i = 0; i < grid_size; ++i) {
        for (int j = 0; j < grid_size; ++j) {
            grid[i][j] = complex.add_vertex();
        }
    }

    // Create edges and triangles
    for (int i = 0; i < grid_size - 1; ++i) {
        for (int j = 0; j < grid_size - 1; ++j) {
            SimplexID tri1 = complex.add_triangle(grid[i][j], grid[i+1][j], grid[i][j+1]);
            SimplexID tri2 = complex.add_triangle(grid[i+1][j+1], grid[i][j+1], grid[i+1][j]);

            complex.set_label(tri1, 0.5 + i * 0.01 + j * 0.01);
            complex.set_label(tri2, 0.5 + i * 0.01 + j * 0.01);
        }
    }

    size_t vertex_count = complex.vertex_count();
    size_t triangle_count = complex.get_simplices_of_dimension(2).size();

    std::cout << "  Created " << vertex_count << " vertices" << std::endl;
    std::cout << "  Created " << triangle_count << " triangles" << std::endl;

    // Save
    bool saved = Persistence::save_labeled(complex, "test_large.bin");
    assert(saved);

    // Load
    auto result = Persistence::load_labeled<double>("test_large.bin");
    assert(result.success);

    // Verify
    assert(result.complex.vertex_count() == vertex_count);
    assert(result.complex.get_simplices_of_dimension(2).size() == triangle_count);

    std::cout << "  File size: " << result.metadata.file_size << " bytes" << std::endl;
    std::cout << "  Bytes per vertex: " << (result.metadata.file_size / vertex_count) << std::endl;

    std::filesystem::remove("test_large.bin");

    std::cout << "✓ Large complex save/load passed" << std::endl;
}

void test_error_handling() {
    std::cout << "Testing error handling..." << std::endl;

    // Test loading non-existent file
    auto result = Persistence::load("nonexistent.bin");
    assert(!result.success);
    assert(!result.error_message.empty());

    std::cout << "  Error message: " << result.error_message << std::endl;

    // Test labeled load non-existent file
    auto result_labeled = Persistence::load_labeled<double>("nonexistent.bin");
    assert(!result_labeled.success);

    std::cout << "✓ Error handling passed" << std::endl;
}

int main() {
    std::cout << "=== Cebu Persistence Tests ===" << std::endl << std::endl;

    try {
        cleanup_test_files();

        test_format_detection();
        test_basic_save_load();
        test_labeled_save_load();
        test_narrative_save_load();
        test_refinement_save_load();
        test_get_metadata();
        test_validate_file();
        test_large_complex();
        test_error_handling();

        cleanup_test_files();

        std::cout << std::endl;
        std::cout << "=====================================" << std::endl;
        std::cout << "All persistence tests passed! ✅" << std::endl;
        std::cout << "=====================================" << std::endl;

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        cleanup_test_files();
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception" << std::endl;
        cleanup_test_files();
        return 1;
    }
}
