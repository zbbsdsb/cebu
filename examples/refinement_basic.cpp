// Basic Refinement Example for Cebu Library
// This example demonstrates basic edge and triangle refinement operations

#include "cebu/refinement.h"
#include <iostream>
#include <iomanip>

using namespace cebu;

void print_complex_info(const SimplicialComplexRefinement<double>& complex, const std::string& title) {
    std::cout << "\n=== " << title << " ===\n";
    std::cout << "Vertices: " << complex.vertex_count() << "\n";
    std::cout << "Edges: " << complex.get_simplices_of_dimension(1).size() << "\n";
    std::cout << "Triangles: " << complex.get_simplices_of_dimension(2).size() << "\n";
    std::cout << "Labeled simplices: " << complex.labeled_count() << "\n";
}

int main() {
    std::cout << "=== Cebu Basic Refinement Example ===\n\n";

    // ============================================================================
    // Example 1: Basic Edge Refinement
    // ============================================================================

    {
        std::cout << "Example 1: Basic Edge Refinement\n";
        std::cout << std::string(40, '=') << "\n\n";

        SimplicialComplexRefinement<double> complex;

        // Create an edge
        VertexID v0 = complex.add_vertex();
        VertexID v1 = complex.add_vertex();
        SimplexID edge = complex.add_edge(v0, v1);

        std::cout << "Created edge " << edge << " between vertices " << v0 << " and " << v1 << "\n";

        // Set label on the edge
        complex.set_label(edge, 0.8);
        std::cout << "Set label: 0.8\n";

        print_complex_info(complex, "Before Refinement");

        // Refine the edge
        RefinementOptions<double> options;
        options.label_strategy = LabelInheritanceStrategy::INHERIT_COPY;

        RefinementResult result = complex.refine_edge(edge, options);

        std::cout << "\nRefinement Result:\n";
        std::cout << "  New vertices created: " << result.new_vertices_count << "\n";
        std::cout << "  New simplices created: " << result.new_simplices_count << "\n";

        print_complex_info(complex, "After Refinement");

        // Check refinement levels
        std::cout << "\nRefinement Levels:\n";
        for (const auto& [parent_id, children] : result.original_to_children) {
            std::cout << "  Original edge " << parent_id << " refined into:\n";
            for (SimplexID child_id : children) {
                int level = complex.get_refinement_level(child_id);
                auto label = complex.get_label(child_id);
                std::cout << "    - Child " << child_id << " (level " << level;
                if (label.has_value()) {
                    std::cout << ", label " << std::fixed << std::setprecision(2) << *label;
                }
                std::cout << ")\n";
            }
        }
    }

    // ============================================================================
    // Example 2: Triangle Refinement
    // ============================================================================

    {
        std::cout << "\n\nExample 2: Triangle Refinement\n";
        std::cout << std::string(40, '=') << "\n\n";

        SimplicialComplexRefinement<double> complex;

        // Create a triangle
        VertexID v0 = complex.add_vertex();
        VertexID v1 = complex.add_vertex();
        VertexID v2 = complex.add_vertex();
        SimplexID tri = complex.add_triangle(v0, v1, v2);

        std::cout << "Created triangle " << tri << " with vertices " << v0 << ", " << v1 << ", " << v2 << "\n";

        // Set label on the triangle
        complex.set_label(tri, 0.9);
        std::cout << "Set label: 0.9\n";

        print_complex_info(complex, "Before Refinement");

        // Refine the triangle with different label inheritance strategies
        std::cout << "\nRefining with INHERIT_INTERPOLATE strategy:\n";

        RefinementOptions<double> options;
        options.label_strategy = LabelInheritanceStrategy::INHERIT_INTERPOLATE;

        RefinementResult result = complex.refine_triangle(tri, options);

        std::cout << "Refinement Result:\n";
        std::cout << "  New vertices created: " << result.new_vertices_count << "\n";
        std::cout << "  New triangles created: " << result.new_simplices_count << "\n";

        print_complex_info(complex, "After Refinement");

        // Show the 4 sub-triangles
        std::cout << "\nSub-triangles created:\n";
        auto children = result.original_to_children[tri];
        for (size_t i = 0; i < children.size(); ++i) {
            SimplexID child_id = children[i];
            const auto& simplex = complex.get_simplex(child_id);
            const auto& vertices = simplex.vertices();

            std::cout << "  Triangle " << child_id << ": vertices [";
            for (size_t j = 0; j < vertices.size(); ++j) {
                if (j > 0) std::cout << ", ";
                std::cout << vertices[j];
            }
            std::cout << "]";

            auto label = complex.get_label(child_id);
            if (label.has_value()) {
                std::cout << " (label: " << std::fixed << std::setprecision(2) << *label << ")";
            }
            std::cout << "\n";
        }
    }

    // ============================================================================
    // Example 3: Custom Label Inheritance
    // ============================================================================

    {
        std::cout << "\n\nExample 3: Custom Label Inheritance\n";
        std::cout << std::string(40, '=') << "\n\n";

        SimplicialComplexRefinement<double> complex;

        VertexID v0 = complex.add_vertex();
        VertexID v1 = complex.add_vertex();
        VertexID v2 = complex.add_vertex();
        SimplexID tri = complex.add_triangle(v0, v1, v2);

        complex.set_label(tri, 1.0);
        std::cout << "Set label: 1.0\n";

        // Refine with custom label function
        RefinementOptions<double> options;
        options.label_strategy = LabelInheritanceStrategy::INHERIT_CUSTOM;
        options.custom_label_func = [](const double& parent, size_t index, size_t total) {
            // Distribute label values: first child gets 100%, second gets 75%, etc.
            double factor = 1.0 - static_cast<double>(index) / static_cast<double>(total);
            return parent * factor;
        };

        std::cout << "Refining with custom label distribution:\n";
        std::cout << "  Child 0: 100% of parent\n";
        std::cout << "  Child 1: 75% of parent\n";
        std::cout << "  Child 2: 50% of parent\n";
        std::cout << "  Child 3: 25% of parent\n";

        RefinementResult result = complex.refine_triangle(tri, options);

        // Show labels
        std::cout << "\nLabel distribution:\n";
        auto children = result.original_to_children[tri];
        for (size_t i = 0; i < children.size(); ++i) {
            SimplexID child_id = children[i];
            auto label = complex.get_label(child_id);
            if (label.has_value()) {
                std::cout << "  Triangle " << child_id << ": " << std::fixed << std::setprecision(2) << *label << "\n";
            }
        }
    }

    // ============================================================================
    // Example 4: Refinement Level Management
    // ============================================================================

    {
        std::cout << "\n\nExample 4: Refinement Level Management\n";
        std::cout << std::string(40, '=') << "\n\n";

        SimplicialComplexRefinement<double> complex;

        // Create a triangle
        VertexID v0 = complex.add_vertex();
        VertexID v1 = complex.add_vertex();
        VertexID v2 = complex.add_vertex();
        SimplexID tri = complex.add_triangle(v0, v1, v2);

        // First refinement (level 0 -> 1)
        RefinementOptions<double> options1;
        options1.current_level = 0;
        options1.max_level = 10;
        auto result1 = complex.refine_triangle(tri, options1);

        std::cout << "First refinement:\n";
        std::cout << "  Created 4 triangles at level 1\n";

        // Get triangles at level 1
        auto level1_tris = complex.get_simplices_at_level(1);
        std::cout << "  Triangles at level 1: " << level1_tris.size() << "\n";

        // Refine one of the sub-triangles (level 1 -> 2)
        if (!level1_tris.empty()) {
            SimplexID sub_tri = level1_tris[0];
            RefinementOptions<double> options2;
            options2.current_level = 1;
            options2.max_level = 10;

            auto result2 = complex.refine_triangle(sub_tri, options2);

            std::cout << "\nSecond refinement (of one sub-triangle):\n";
            std::cout << "  Created 4 triangles at level 2\n";

            auto level2_tris = complex.get_simplices_at_level(2);
            std::cout << "  Triangles at level 2: " << level2_tris.size() << "\n";
        }

        // Show all triangles by level
        std::cout << "\nAll triangles by level:\n";
        auto all_tris = complex.get_simplices_of_dimension(2);
        for (SimplexID tri_id : all_tris) {
            int level = complex.get_refinement_level(tri_id);
            std::cout << "  Triangle " << tri_id << ": level " << level << "\n";
        }
    }

    // ============================================================================
    // Example 5: Coarsening
    // ============================================================================

    {
        std::cout << "\n\nExample 5: Coarsening\n";
        std::cout << std::string(40, '=') << "\n\n";

        SimplicialComplexRefinement<double> complex;

        // Create and refine an edge
        VertexID v0 = complex.add_vertex();
        VertexID v1 = complex.add_vertex();
        SimplexID edge = complex.add_edge(v0, v1);

        RefinementResult result = complex.refine_edge(edge);

        print_complex_info(complex, "After Refinement");

        // Find the midpoint vertex (new vertex)
        auto new_vertices = result.new_vertex_parent_edge;
        if (!new_vertices.empty()) {
            VertexID midpoint = new_vertices.begin()->first;

            std::cout << "\nMidpoint vertex: " << midpoint << "\n";
            std::cout << "Is refinement midpoint: " << (complex.is_refinement_midpoint(midpoint) ? "Yes" : "No") << "\n";

            // Coarsen the edge
            RefinementOptions<double> options;
            bool success = complex.coarsen_edge(midpoint, options);

            std::cout << "\nCoarsening result: " << (success ? "Success" : "Failed") << "\n";

            print_complex_info(complex, "After Coarsening");
        }
    }

    // ============================================================================
    // Summary
    // ============================================================================

    std::cout << "\n\n=== Summary ===\n";
    std::cout << "This example demonstrated:\n";
    std::cout << "  1. Basic edge refinement\n";
    std::cout << "  2. Triangle refinement\n";
    std::cout << "  3. Custom label inheritance\n";
    std::cout << "  4. Refinement level management\n";
    std::cout << "  5. Coarsening operations\n";
    std::cout << "\nRefinement enables adaptive mesh resolution and multi-resolution analysis.\n";

    return 0;
}
