#include "cebu/persistence.h"
#include "cebu/refinement.h"
#include "cebu/simplicial_complex_narrative.h"
#include <iostream>

using namespace cebu;

void example_basic_save_load() {
    std::cout << "=== Example: Basic Save/Load ===" << std::endl;

    // Create a simple complex
    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();

    complex.add_edge(v0, v1);
    complex.add_edge(v1, v2);
    complex.add_triangle(v0, v1, v2);

    std::cout << "Created complex with " << complex.vertex_count() << " vertices" << std::endl;

    // Save to file
    PersistenceOptions options;
    options.format = FileFormat::BINARY;

    bool saved = Persistence::save(complex, "example_basic.bin", options);
    if (saved) {
        std::cout << "Successfully saved to example_basic.bin" << std::endl;
    } else {
        std::cerr << "Failed to save" << std::endl;
        return;
    }

    // Load from file
    auto result = Persistence::load("example_basic.bin", options);

    if (result.success) {
        std::cout << "Successfully loaded from example_basic.bin" << std::endl;
        std::cout << "  Restored " << result.complex.vertex_count() << " vertices" << std::endl;
        std::cout << "  File size: " << result.metadata.file_size << " bytes" << std::endl;
    } else {
        std::cerr << "Failed to load: " << result.error_message << std::endl;
    }

    std::cout << std::endl;
}

void example_labeled_with_options() {
    std::cout << "=== Example: Labeled Complex with Options ===" << std::endl;

    // Create a labeled complex
    SimplicialComplexLabeled<double> complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();

    SimplexID edge0 = complex.add_edge(v0, v1);
    SimplexID edge1 = complex.add_edge(v1, v2);
    SimplexID tri = complex.add_triangle(v0, v1, v2);

    complex.set_label(v0, 0.2);
    complex.set_label(v1, 0.5);
    complex.set_label(v2, 0.8);
    complex.set_label(edge0, 0.35);
    complex.set_label(edge1, 0.65);
    complex.set_label(tri, 0.5);

    std::cout << "Created labeled complex" << std::endl;

    // Save with custom options
    PersistenceOptions options;
    options.format = FileFormat::BINARY;
    options.compression = Compression::NONE; // Could be ZLIB if implemented
    options.include_metadata = true;

    bool saved = Persistence::save_labeled(complex, "example_labeled.bin", options);
    if (!saved) {
        std::cerr << "Failed to save" << std::endl;
        return;
    }

    // Get metadata without loading
    auto metadata = Persistence::get_metadata("example_labeled.bin");
    std::cout << "File metadata:" << std::endl;
    std::cout << "  Cebu version: " << metadata.cebu_version << std::endl;
    std::cout << "  Simplex count: " << metadata.simplex_count << std::endl;
    std::cout << "  File size: " << metadata.file_size << " bytes" << std::endl;

    // Load and verify
    auto result = Persistence::load_labeled<double>("example_labeled.bin");

    if (result.success) {
        std::cout << "Successfully loaded" << std::endl;

        // Verify labels
        auto label_tri = result.complex.get_label(tri);
        if (label_tri) {
            std::cout << "  Triangle label: " << *label_tri << std::endl;
        }
    }

    std::cout << std::endl;
}

void example_narrative_persistence() {
    std::cout << "=== Example: Narrative Complex Persistence ===" << std::endl;

    // Create a narrative complex
    SimplicialComplexNarrative<double> complex(0.0, 10.0);

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();

    SimplexID edge0 = complex.add_edge(v0, v1);
    SimplexID edge1 = complex.add_edge(v1, v2);
    SimplexID tri = complex.add_triangle(v0, v1, v2);

    // Set labels representing narrative elements
    complex.set_label(edge0, 0.3); // Beginning - low tension
    complex.set_label(edge1, 0.8); // Climax - high tension
    complex.set_label(tri, 0.6);   // Resolution - medium tension

    // Add timeline milestones
    complex.timeline().add_milestone(2.0, "Introduction");
    complex.timeline().add_milestone(5.0, "Climax");
    complex.timeline().add_milestone(8.0, "Resolution");

    // Add story events
    AbsurdityContext impact1;
    impact1.surprisal = 0.7;
    impact1.narrative_tension = 0.6;
    impact1.logical_deviation = 0.4;

    complex.add_event("Plot twist", 2.5, {edge0}, impact1);

    AbsurdityContext impact2;
    impact2.surprisal = 0.9;
    impact2.narrative_tension = 0.95;
    impact2.logical_deviation = 0.8;

    complex.add_event("Major revelation", 5.0, {tri}, impact2);

    // Evolve to a point in time
    complex.evolve_to(5.0);

    std::cout << "Created narrative complex" << std::endl;
    std::cout << "  Milestones: " << complex.timeline().get_milestones().size() << std::endl;
    std::cout << "  Events: " << complex.events().get_all_events().size() << std::endl;
    std::cout << "  Current time: " << complex.current_time() << std::endl;

    // Save
    bool saved = Persistence::save_narrative(complex, "example_narrative.bin");
    if (!saved) {
        std::cerr << "Failed to save" << std::endl;
        return;
    }

    std::cout << "Saved narrative to example_narrative.bin" << std::endl;

    // Load
    auto result = Persistence::load_narrative<double>("example_narrative.bin");

    if (result.success) {
        std::cout << "Successfully loaded narrative" << std::endl;
        std::cout << "  Milestones: " << result.complex.timeline().get_milestones().size() << std::endl;
        std::cout << "  Events: " << result.complex.events().get_all_events().size() << std::endl;

        // Verify an event
        auto events = result.complex.events().get_all_events();
        for (const auto& [id, event] : events) {
            std::cout << "  Event: " << event.description
                      << " at time " << event.timestamp << std::endl;
        }
    }

    std::cout << std::endl;
}

void example_refinement_persistence() {
    std::cout << "=== Example: Refinement Complex Persistence ===" << std::endl;

    // Create a refinement complex
    SimplicialComplexRefinement<double> complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();

    SimplexID tri = complex.add_triangle(v0, v1, v2);

    // Set label and refinement level
    complex.set_label(tri, 0.9);
    complex.set_refinement_level(tri, 3);

    std::cout << "Created refinement complex" << std::endl;
    std::cout << "  Triangle refinement level: "
              << complex.get_refinement_level(tri) << std::endl;

    // Save
    bool saved = Persistence::save_refinement(complex, "example_refinement.bin");
    if (!saved) {
        std::cerr << "Failed to save" << std::endl;
        return;
    }

    std::cout << "Saved refinement to example_refinement.bin" << std::endl;

    // Load
    auto result = Persistence::load_refinement<double>("example_refinement.bin");

    if (result.success) {
        std::cout << "Successfully loaded refinement" << std::endl;
        int level = result.complex.get_refinement_level(tri);
        std::cout << "  Triangle refinement level: " << level << std::endl;
        assert(level == 3);
    }

    std::cout << std::endl;
}

void example_format_detection() {
    std::cout << "=== Example: Format Detection ===" << std::endl;

    SimplicialComplex complex;
    complex.add_vertex();
    complex.add_vertex();
    complex.add_edge(0, 1);

    // Save with different extensions
    Persistence::save(complex, "data.bin");    // Binary
    Persistence::save(complex, "data.ceb");    // Binary (custom)
    // Note: JSON saving would need JsonSerializer implementation

    std::cout << "File format detection:" << std::endl;
    std::cout << "  data.bin -> " << (int)Persistence::detect_format("data.bin") << std::endl;
    std::cout << "  data.ceb -> " << (int)Persistence::detect_format("data.ceb") << std::endl;
    std::cout << "  data.json -> " << (int)Persistence::detect_format("data.json") << std::endl;

    std::cout << std::endl;
}

void example_validation() {
    std::cout << "=== Example: File Validation ===" << std::endl;

    SimplicialComplex complex;
    complex.add_vertex();
    complex.add_vertex();
    complex.add_edge(0, 1);

    Persistence::save(complex, "valid_file.bin");

    bool is_valid = Persistence::validate_file("valid_file.bin");
    std::cout << "valid_file.bin is " << (is_valid ? "valid" : "invalid") << std::endl;

    // Create invalid file
    {
        std::ofstream out("invalid_file.bin");
        out << "This is not a valid Cebu file";
        out.close();
    }

    bool is_invalid = !Persistence::validate_file("invalid_file.bin");
    std::cout << "invalid_file.bin is " << (is_invalid ? "invalid" : "valid") << std::endl;

    std::cout << std::endl;
}

void example_workflow() {
    std::cout << "=== Example: Complete Workflow ===" << std::endl;

    // 1. Create and save initial state
    SimplicialComplexNarrative<double> complex(0.0, 20.0);

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    SimplexID tri = complex.add_triangle(v0, v1, v2);

    complex.set_label(tri, 0.5);
    complex.timeline().add_milestone(5.0, "Checkpoint 1");

    Persistence::save_narrative(complex, "workflow_v1.bin");
    std::cout << "Saved version 1" << std::endl;

    // 2. Modify and save as version 2
    complex.set_label(tri, 0.8);
    complex.timeline().add_milestone(10.0, "Checkpoint 2");

    AbsurdityContext impact;
    impact.surprisal = 0.9;
    complex.add_event("Major change", 10.0, {tri}, impact);

    Persistence::save_narrative(complex, "workflow_v2.bin");
    std::cout << "Saved version 2" << std::endl;

    // 3. Load version 1
    auto v1_result = Persistence::load_narrative<double>("workflow_v1.bin");
    if (v1_result.success) {
        auto v1_label = v1_result.complex.get_label(tri);
        std::cout << "Version 1 label: " << *v1_label << std::endl;
    }

    // 4. Load version 2
    auto v2_result = Persistence::load_narrative<double>("workflow_v2.bin");
    if (v2_result.success) {
        auto v2_label = v2_result.complex.get_label(tri);
        std::cout << "Version 2 label: " << *v2_label << std::endl;
    }

    std::cout << std::endl;
}

int main() {
    std::cout << "=== Cebu Persistence Examples ===" << std::endl << std::endl;

    try {
        example_basic_save_load();
        example_labeled_with_options();
        example_narrative_persistence();
        example_refinement_persistence();
        example_format_detection();
        example_validation();
        example_workflow();

        std::cout << "=== All examples completed successfully! ===" << std::endl;

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Example failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
