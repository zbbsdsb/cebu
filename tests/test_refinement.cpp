#include "cebu/refinement.h"
#include "cebu/serialization.h"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace cebu;

void test_edge_refinement() {
    std::cout << "Testing edge refinement..." << std::endl;
    
    SimplicialComplexRefinement<double> complex;
    
    // Create an edge
    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    SimplexID edge = complex.add_edge(v0, v1);
    
    // Set a label on the edge
    complex.set_label(edge, 1.5);
    
    // Refine the edge
    RefinementOptions<double> options;
    options.current_level = 0;
    RefinementResult result = complex.refine_edge(edge, options);
    
    assert(result.new_simplices_count == 2);
    assert(result.new_vertices_count == 1);
    assert(result.original_to_children.size() == 1);
    
    // Check that original edge no longer exists
    assert(!complex.has_simplex(edge));
    
    // Check that new vertices and edges exist
    assert(complex.vertex_count() == 3);
    assert(complex.get_simplices_of_dimension(1).size() == 2);
    
    // Check labels were inherited
    auto children = result.original_to_children[edge];
    assert(children.size() == 2);
    assert(complex.has_label(children[0]));
    assert(complex.has_label(children[1]));
    
    std::cout << "✓ Edge refinement passed" << std::endl;
}

void test_triangle_refinement() {
    std::cout << "Testing triangle refinement..." << std::endl;
    
    SimplicialComplexRefinement<double> complex;
    
    // Create a triangle
    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    SimplexID tri = complex.add_triangle(v0, v1, v2);
    
    // Set a label
    complex.set_label(tri, 0.8);
    
    // Refine the triangle
    RefinementOptions<double> options;
    options.current_level = 0;
    RefinementResult result = complex.refine_triangle(tri, options);
    
    assert(result.new_simplices_count == 4);
    assert(result.new_vertices_count == 3);
    assert(result.original_to_children.size() == 1);
    
    // Check original triangle is gone
    assert(!complex.has_simplex(tri));
    
    // Check new structure
    assert(complex.vertex_count() == 6); // 3 original + 3 new
    assert(complex.get_simplices_of_dimension(2).size() == 4);
    
    // Check labels
    auto children = result.original_to_children[tri];
    assert(children.size() == 4);
    for (SimplexID child : children) {
        assert(complex.has_label(child));
    }
    
    std::cout << "✓ Triangle refinement passed" << std::endl;
}

void test_refinement_levels() {
    std::cout << "Testing refinement levels..." << std::endl;
    
    SimplicialComplexRefinement<double> complex;
    
    // Create and refine a triangle
    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    SimplexID tri = complex.add_triangle(v0, v1, v2);
    
    assert(complex.get_refinement_level(tri) == 0);
    
    // Refine
    RefinementOptions<double> options;
    options.current_level = 0;
    RefinementResult result = complex.refine_triangle(tri, options);
    
    // Check children have level 1
    auto children = result.original_to_children[tri];
    for (SimplexID child : children) {
        assert(complex.get_refinement_level(child) == 1);
    }
    
    // Get all simplices at level 1
    auto level1_simplices = complex.get_simplices_at_level(1);
    assert(level1_simplices.size() == 4);
    
    std::cout << "✓ Refinement levels passed" << std::endl;
}

void test_edge_coarsening() {
    std::cout << "Testing edge coarsening..." << std::endl;
    
    SimplicialComplexRefinement<double> complex;
    
    // Create and refine an edge
    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    EdgeID edge = complex.add_edge(v0, v1);
    
    RefinementOptions<double> options;
    options.current_level = 0;
    RefinementResult result = complex.refine_edge(edge, options);
    
    // Now coarsen it back
    // Find the midpoint vertex
    auto vertices = complex.get_vertices_of_simplex(result.original_to_children[edge][0]);
    VertexID mid = static_cast<VertexID>(vertices[1]);
    
    // Check it's a refinement midpoint
    assert(complex.is_refinement_midpoint(mid));
    
    // Coarsen
    bool success = complex.coarsen_edge(mid, options);
    assert(success);
    
    // Check structure is restored
    assert(complex.vertex_count() == 2);
    assert(complex.get_simplices_of_dimension(1).size() == 1);
    
    std::cout << "✓ Edge coarsening passed" << std::endl;
}

void test_triangle_coarsening() {
    std::cout << "Testing triangle coarsening..." << std::endl;
    
    SimplicialComplexRefinement<double> complex;
    
    // Create and refine a triangle
    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    SimplexID tri = complex.add_triangle(v0, v1, v2);
    
    RefinementOptions<double> options;
    options.current_level = 0;
    RefinementResult result = complex.refine_triangle(tri, options);
    
    // Find the center vertex (connects to all 3 midpoints)
    // The center is one of the new vertices
    auto all_triangles = complex.get_simplices_of_dimension(2);
    
    // The center triangle is the one with all new vertices
    SimplexID center_tri = result.original_to_children[tri][3];
    auto center_verts = complex.get_vertices_of_simplex(center_tri);
    VertexID center = static_cast<VertexID>(center_verts[0]);
    
    // Coarsen
    bool success = complex.coarsen_triangle(center, options);
    assert(success);
    
    // Check structure is restored
    assert(complex.vertex_count() == 3);
    assert(complex.get_simplices_of_dimension(2).size() == 1);
    
    std::cout << "✓ Triangle coarsening passed" << std::endl;
}

void test_max_level_limit() {
    std::cout << "Testing max level limit..." << std::endl;
    
    SimplicialComplexRefinement<double> complex;
    
    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    SimplexID tri = complex.add_triangle(v0, v1, v2);
    
    RefinementOptions<double> options;
    options.max_level = 2;
    
    // First refinement (level 0 -> 1)
    options.current_level = 0;
    auto result1 = complex.refine_triangle(tri, options);
    assert(result1.new_simplices_count == 4);
    
    // Try to refine a child (level 1 -> 2)
    SimplexID child = result1.original_to_children[tri][0];
    options.current_level = 1;
    auto result2 = complex.refine_triangle(child, options);
    assert(result2.new_simplices_count == 4);
    
    // Try to refine a grandchild (level 2 -> 3, should fail)
    SimplexID grandchild = result2.original_to_children[child][0];
    options.current_level = 2;
    auto result3 = complex.refine_triangle(grandchild, options);
    assert(result3.new_simplices_count == 0);
    
    std::cout << "✓ Max level limit passed" << std::endl;
}

void test_label_inheritance_strategies() {
    std::cout << "Testing label inheritance strategies..." << std::endl;
    
    SimplicialComplexRefinement<double> complex;
    
    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    SimplexID tri = complex.add_triangle(v0, v1, v2);
    complex.set_label(tri, 1.0);
    
    // Test INHERIT_COPY
    RefinementOptions<double> options1;
    options1.label_strategy = LabelInheritanceStrategy::INHERIT_COPY;
    options1.current_level = 0;
    
    auto result1 = complex.refine_triangle(tri, options1);
    auto children1 = result1.original_to_children[tri];
    for (SimplexID child : children1) {
        assert(std::abs(complex.get_label(child) - 1.0) < 0.001);
    }
    
    std::cout << "✓ Label inheritance strategies passed" << std::endl;
}

void test_custom_label_function() {
    std::cout << "Testing custom label function..." << std::endl;
    
    SimplicialComplexRefinement<double> complex;
    
    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    SimplexID edge = complex.add_edge(v0, v1);
    complex.set_label(edge, 10.0);
    
    // Custom function: divide label by index + 1
    RefinementOptions<double> options;
    options.label_strategy = LabelInheritanceStrategy::INHERIT_CUSTOM;
    options.custom_label_func = [](const double& label, size_t index, size_t total) {
        return label / static_cast<double>(index + 1);
    };
    options.current_level = 0;
    
    auto result = complex.refine_edge(edge, options);
    auto children = result.original_to_children[edge];
    
    assert(std::abs(complex.get_label(children[0]) - 10.0) < 0.001);
    assert(std::abs(complex.get_label(children[1]) - 5.0) < 0.001);
    
    std::cout << "✓ Custom label function passed" << std::endl;
}

void test_adaptive_refinement() {
    std::cout << "Testing adaptive refinement..." << std::endl;
    
    SimplicialComplexRefinement<double> complex;
    
    // Create multiple edges with different labels
    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    VertexID v3 = complex.add_vertex();
    
    SimplexID e0 = complex.add_edge(v0, v1);
    SimplexID e1 = complex.add_edge(v1, v2);
    SimplexID e2 = complex.add_edge(v2, v3);
    
    complex.set_label(e0, 0.3);  // Low
    complex.set_label(e1, 0.8);  // High
    complex.set_label(e2, 0.9);  // High
    
    // Adaptive refinement: refine edges with label > 0.5
    RefinementOptions<double> options;
    options.current_level = 0;
    
    auto result = complex.adaptive_refine(
        [](const double& label, SimplexID) {
            return label > 0.5;
        },
        options
    );
    
    // Should have refined 2 edges
    assert(result.original_to_children.size() == 2);
    
    std::cout << "✓ Adaptive refinement passed" << std::endl;
}

void test_refinement_result_tracking() {
    std::cout << "Testing refinement result tracking..." << std::endl;
    
    SimplicialComplexRefinement<double> complex;
    
    // Create and refine multiple simplices
    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    
    SimplexID edge = complex.add_edge(v0, v1);
    SimplexID tri = complex.add_triangle(v0, v1, v2);
    
    complex.set_label(edge, 1.0);
    complex.set_label(tri, 2.0);
    
    RefinementOptions<double> options;
    options.current_level = 0;
    
    auto result = complex.refine_region({edge, tri}, options);
    
    assert(result.new_simplices_count == 6);  // 2 from edge + 4 from triangle
    assert(result.new_vertices_count == 4);   // 1 from edge + 3 from triangle
    assert(result.original_to_children.size() == 2);
    assert(result.new_vertex_parent_edge.size() == 1);
    
    std::cout << "✓ Refinement result tracking passed" << std::endl;
}

void test_region_coarsening() {
    std::cout << "Testing region coarsening..." << std::endl;
    
    SimplicialComplexRefinement<double> complex;
    
    // Create and refine two edges
    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    
    SimplexID e0 = complex.add_edge(v0, v1);
    SimplexID e1 = complex.add_edge(v1, v2);
    
    complex.set_label(e0, 1.0);
    complex.set_label(e1, 2.0);
    
    RefinementOptions<double> options;
    options.current_level = 0;
    
    auto result = complex.refine_region({e0, e1}, options);
    
    // Now coarsen based on label predicate
    size_t coarsened = complex.coarsen_region(
        [](const double& label, SimplexID id) {
            // Coarsen if label is 1.0
            return std::abs(label - 1.0) < 0.01;
        },
        options
    );
    
    // One of the refined edges should be coarsened
    assert(coarsened >= 1);
    
    std::cout << "✓ Region coarsening passed" << std::endl;
}

int main() {
    std::cout << "=== Cebu Refinement Tests ===" << std::endl << std::endl;
    
    try {
        test_edge_refinement();
        test_triangle_refinement();
        test_refinement_levels();
        test_edge_coarsening();
        test_triangle_coarsening();
        test_max_level_limit();
        test_label_inheritance_strategies();
        test_custom_label_function();
        test_adaptive_refinement();
        test_refinement_result_tracking();
        test_region_coarsening();
        
        std::cout << std::endl;
        std::cout << "=====================================" << std::endl;
        std::cout << "All refinement tests passed! ✅" << std::endl;
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
