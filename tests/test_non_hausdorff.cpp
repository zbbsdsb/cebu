#include "cebu/simplicial_complex_non_hausdorff.h"
#include <cassert>
#include <iostream>

using namespace cebu;

void test_basic_construction() {
    std::cout << "Testing basic construction..." << std::endl;

    SimplicialComplexNonHausdorff complex;

    // Add some vertices
    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();

    assert(complex.vertex_count() == 3);
    assert(complex.equivalence_class_count() == 0);

    std::cout << "✓ Basic construction passed" << std::endl;
}

void test_glue_vertices() {
    std::cout << "Testing glue_vertices..." << std::endl;

    SimplicialComplexNonHausdorff complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    VertexID v3 = complex.add_vertex();

    // Glue v1 and v2
    SimplexID rep = complex.glue_vertices(v1, v2);
    assert(complex.are_glued(v1, v2));
    assert(complex.is_glued(v1));
    assert(complex.is_glued(v2));
    assert(!complex.is_glued(v0));
    assert(!complex.is_glued(v3));

    // Both should have same representative
    assert(complex.get_representative(v1) == complex.get_representative(v2));

    std::cout << "✓ glue_vertices passed" << std::endl;
}

void test_glue_simplices() {
    std::cout << "Testing glue..." << std::endl;

    SimplicialComplexNonHausdorff complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    VertexID v3 = complex.add_vertex();
    VertexID v4 = complex.add_vertex();
    VertexID v5 = complex.add_vertex();

    // Create triangles
    SimplexID tri1 = complex.add_triangle(v0, v1, v2);
    SimplexID tri2 = complex.add_triangle(v3, v4, v5);

    // Glue the triangles
    SimplexID rep = complex.glue(tri1, tri2);
    assert(complex.are_glued(tri1, tri2));
    assert(complex.is_glued(tri1));

    // Get equivalence class
    auto members = complex.get_equivalence_class(tri1);
    assert(members.size() == 2);

    std::cout << "✓ glue passed" << std::endl;
}

void test_get_equivalence_class() {
    std::cout << "Testing get_equivalence_class..." << std::endl;

    SimplicialComplexNonHausdorff complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    VertexID v3 = complex.add_vertex();
    VertexID v4 = complex.add_vertex();

    // Glue v1, v2, v3, v4 together
    complex.glue_vertices(v1, v2);
    complex.glue_vertices(v2, v3);
    complex.glue_vertices(v3, v4);

    // Get class from v1
    auto members = complex.get_equivalence_class(v1);
    assert(members.size() == 4);

    // Get class from any member should give same result
    auto members2 = complex.get_equivalence_class(v3);
    assert(members.size() == members2.size());

    // v0 should be alone
    auto members0 = complex.get_equivalence_class(v0);
    assert(members0.size() == 1);

    std::cout << "✓ get_equivalence_class passed" << std::endl;
}

void test_separate() {
    std::cout << "Testing separate..." << std::endl;

    SimplicialComplexNonHausdorff complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();

    // Glue v1 and v2
    complex.glue_vertices(v1, v2);
    assert(complex.are_glued(v1, v2));

    // Separate v2
    bool separated = complex.separate(v2);
    assert(separated);
    assert(!complex.are_glued(v1, v2));
    assert(complex.class_size(v2) == 1);

    // v1 should be alone now
    assert(!complex.is_glued(v1));

    // Try separating again (should return false)
    separated = complex.separate(v2);
    assert(!separated);

    std::cout << "✓ separate passed" << std::endl;
}

void test_adjacent_with_gluing() {
    std::cout << "Testing get_adjacent_with_gluing..." << std::endl;

    SimplicialComplexNonHausdorff complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    VertexID v3 = complex.add_vertex();
    VertexID v4 = complex.add_vertex();
    VertexID v5 = complex.add_vertex();

    // Create edges
    SimplexID e0 = complex.add_edge(v0, v1);
    SimplexID e1 = complex.add_edge(v1, v2);
    SimplexID e2 = complex.add_edge(v2, v3);
    SimplexID e3 = complex.add_edge(v3, v4);
    SimplexID e4 = complex.add_edge(v4, v5);

    // Glue v2 and v4
    complex.glue_vertices(v2, v4);

    // Get adjacent simplices for the glued group (v2/v4)
    auto adjacent = complex.get_adjacent_with_gluing(v2);
    assert(adjacent.size() >= 2); // Should include edges connected to v2 or v4

    std::cout << "✓ get_adjacent_with_gluing passed" << std::endl;
}

void test_simplices_containing_vertex_with_gluing() {
    std::cout << "Testing get_simplices_containing_vertex_with_gluing..." << std::endl;

    SimplicialComplexNonHausdorff complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    VertexID v3 = complex.add_vertex();
    VertexID v4 = complex.add_vertex();

    // Create edges
    SimplexID e0 = complex.add_edge(v0, v1);
    SimplexID e1 = complex.add_edge(v1, v2);
    SimplexID e2 = complex.add_edge(v2, v3);
    SimplexID e3 = complex.add_edge(v3, v4);

    // Glue v1 and v3
    complex.glue_vertices(v1, v3);

    // Get simplices containing the glued group
    auto containing = complex.get_simplices_containing_vertex_with_gluing(v1);
    assert(containing.size() >= 3); // e0, e1, e2, e3 all touch v1 or v3

    std::cout << "✓ get_simplices_containing_vertex_with_gluing passed" << std::endl;
}

void test_labeled_complex_with_gluing() {
    std::cout << "Testing SimplicialComplexNonHausdorffLabeled..." << std::endl;

    SimplicialComplexNonHausdorffLabeled<double> complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();

    // Set different labels on v0 and v1
    complex.set_label(v0, 0.5);
    complex.set_label(v1, 0.8);

    assert(complex.get_label(v0) == 0.5);
    assert(complex.get_label(v1) == 0.8);

    // Glue v0 and v1
    complex.glue_vertices(v0, v1);

    // Labels should still be different (glued simplices keep their own labels)
    assert(complex.get_label(v0) == 0.5);
    assert(complex.get_label(v1) == 0.8);

    // Get all labels from class
    auto labels = complex.get_class_labels(v0);
    assert(labels.size() == 2);
    assert(labels[v0] == 0.5);
    assert(labels[v1] == 0.8);

    std::cout << "✓ SimplicialComplexNonHausdorffLabeled passed" << std::endl;
}

void test_removing_glued_simplices() {
    std::cout << "Testing removal of glued simplices..." << std::endl;

    SimplicialComplexNonHausdorff complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    VertexID v3 = complex.add_vertex();

    // Glue v1, v2, v3
    complex.glue_vertices(v1, v2);
    complex.glue_vertices(v2, v3);

    assert(complex.class_size(v1) == 3);

    // Remove v2 (middle of chain)
    complex.remove_vertex(v2, false);

    // v1 and v3 should still be glued
    assert(complex.are_glued(v1, v3));
    assert(complex.class_size(v1) == 2);

    std::cout << "✓ Removal of glued simplices passed" << std::endl;
}

void test_multiple_independent_classes() {
    std::cout << "Testing multiple independent equivalence classes..." << std::endl;

    SimplicialComplexNonHausdorff complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    VertexID v3 = complex.add_vertex();
    VertexID v4 = complex.add_vertex();
    VertexID v5 = complex.add_vertex();

    // Create two independent glued groups
    complex.glue_vertices(v0, v1);
    complex.glue_vertices(v1, v2);
    complex.glue_vertices(v3, v4);
    complex.glue_vertices(v4, v5);

    assert(complex.equivalence_class_count() == 2);

    // Verify independence
    assert(complex.are_glued(v0, v2));
    assert(complex.are_glued(v3, v5));
    assert(!complex.are_glued(v0, v3));
    assert(!complex.are_glued(v1, v4));

    std::cout << "✓ Multiple independent classes passed" << std::endl;
}

void test_transitive_gluing() {
    std::cout << "Testing transitive gluing..." << std::endl;

    SimplicialComplexNonHausdorff complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    VertexID v3 = complex.add_vertex();
    VertexID v4 = complex.add_vertex();

    // Chain gluing: v0-v1-v2-v3-v4
    complex.glue_vertices(v0, v1);
    complex.glue_vertices(v1, v2);
    complex.glue_vertices(v2, v3);
    complex.glue_vertices(v3, v4);

    // All should be in same class
    assert(complex.are_glued(v0, v4));
    assert(complex.class_size(v0) == 5);

    // All should have same representative
    SimplexID rep = complex.get_representative(v0);
    assert(complex.get_representative(v1) == rep);
    assert(complex.get_representative(v2) == rep);
    assert(complex.get_representative(v3) == rep);
    assert(complex.get_representative(v4) == rep);

    std::cout << "✓ Transitive gluing passed" << std::endl;
}

int main() {
    std::cout << "=== Non-Hausdorff Topology Tests ===" << std::endl << std::endl;

    test_basic_construction();
    test_glue_vertices();
    test_glue_simplices();
    test_get_equivalence_class();
    test_separate();
    test_adjacent_with_gluing();
    test_simplices_containing_vertex_with_gluing();
    test_labeled_complex_with_gluing();
    test_removing_glued_simplices();
    test_multiple_independent_classes();
    test_transitive_gluing();

    std::cout << std::endl;
    std::cout << "=== All Non-Hausdorff Tests Passed! ===" << std::endl;
    return 0;
}
