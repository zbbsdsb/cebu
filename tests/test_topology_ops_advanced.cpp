#include "cebu/topology_operations.h"
#include "cebu/simplicial_complex.h"
#include <iostream>
#include <cassert>

using namespace cebu;

void test_glue_simplices_by_boundary_basic() {
    std::cout << "Testing glue_simplices_by_boundary (basic)..." << std::endl;

    SimplicialComplex complex;

    // Create two triangles sharing an edge
    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    VertexID v3 = complex.add_vertex();

    SimplexID tri1 = complex.add_triangle(v0, v1, v2);
    SimplexID tri2 = complex.add_triangle(v1, v2, v3);

    // Glue triangles by identifying the shared edge
    std::vector<std::pair<VertexID, VertexID>> mapping = {
        {v1, v1},
        {v2, v2}
    };

    SimplexID result = TopologyOperations::glue_simplices_by_boundary(
        complex, tri1, tri2, mapping);

    assert(result == tri2);
    assert(complex.simplex_count() == 4); // 4 vertices + 2 triangles

    std::cout << "  PASSED" << std::endl;
}

void test_glue_simplices_by_boundary_invalid_simplex() {
    std::cout << "Testing glue_simplices_by_boundary (invalid simplex)..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    SimplexID tri = complex.add_triangle(v0, complex.add_vertex(), complex.add_vertex());

    std::vector<std::pair<VertexID, VertexID>> mapping;

    bool threw = false;
    try {
        TopologyOperations::glue_simplices_by_boundary(complex, tri, 999, mapping);
    } catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);

    std::cout << "  PASSED" << std::endl;
}

void test_glue_simplices_by_boundary_same_simplex() {
    std::cout << "Testing glue_simplices_by_boundary (same simplex)..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    SimplexID tri = complex.add_triangle(v0, complex.add_vertex(), complex.add_vertex());

    std::vector<std::pair<VertexID, VertexID>> mapping;

    bool threw = false;
    try {
        TopologyOperations::glue_simplices_by_boundary(complex, tri, tri, mapping);
    } catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);

    std::cout << "  PASSED" << std::endl;
}

void test_batch_glue_vertices_basic() {
    std::cout << "Testing batch_glue_vertices (basic)..." << std::endl;

    SimplicialComplex complex;

    // Create vertices
    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    VertexID v3 = complex.add_vertex();

    // Create edges
    complex.add_edge(v0, v1);
    complex.add_edge(v1, v2);
    complex.add_edge(v2, v3);

    // Batch glue v0->v1, v2->v3
    std::vector<std::pair<VertexID, VertexID>> gluings = {
        {v0, v1},
        {v2, v3}
    };

    auto results = TopologyOperations::batch_glue_vertices(complex, gluings);

    assert(results.size() == 2);
    assert(complex.simplex_count() == 4); // 2 vertices + 2 edges

    std::cout << "  PASSED" << std::endl;
}

void test_batch_glue_vertices_invalid_vertex() {
    std::cout << "Testing batch_glue_vertices (invalid vertex)..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();

    std::vector<std::pair<VertexID, VertexID>> gluings = {
        {v0, v1},
        {v1, 999}  // Invalid vertex
    };

    bool threw = false;
    try {
        TopologyOperations::batch_glue_vertices(complex, gluings);
    } catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);

    std::cout << "  PASSED" << std::endl;
}

void test_compute_euler_characteristic_point() {
    std::cout << "Testing compute_euler_characteristic (point)..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();

    int euler = TopologyOperations::compute_euler_characteristic(complex);
    assert(euler == 1); // χ = V = 1

    std::cout << "  PASSED" << std::endl;
}

void test_compute_euler_characteristic_edge() {
    std::cout << "Testing compute_euler_characteristic (edge)..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    complex.add_edge(v0, v1);

    int euler = TopologyOperations::compute_euler_characteristic(complex);
    assert(euler == 1); // χ = V - E = 2 - 1 = 1

    std::cout << "  PASSED" << std::endl;
}

void test_compute_euler_characteristic_triangle() {
    std::cout << "Testing compute_euler_characteristic (triangle)..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    complex.add_triangle(v0, v1, v2);

    int euler = TopologyOperations::compute_euler_characteristic(complex);
    assert(euler == 1); // χ = V - E + F = 3 - 3 + 1 = 1

    std::cout << "  PASSED" << std::endl;
}

void test_compute_euler_characteristic_two_triangles() {
    std::cout << "Testing compute_euler_characteristic (two triangles)..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    VertexID v3 = complex.add_vertex();

    complex.add_triangle(v0, v1, v2);
    complex.add_triangle(v1, v2, v3);

    int euler = TopologyOperations::compute_euler_characteristic(complex);
    // χ = V - E + F = 4 - 5 + 2 = 1
    assert(euler == 1);

    std::cout << "  PASSED" << std::endl;
}

void test_is_manifold_only_vertices() {
    std::cout << "Testing is_manifold (only vertices)..." << std::endl;

    SimplicialComplex complex;

    complex.add_vertex();
    complex.add_vertex();
    complex.add_vertex();

    assert(TopologyOperations::is_manifold(complex) == true);

    std::cout << "  PASSED" << std::endl;
}

void test_is_manifold_single_edge() {
    std::cout << "Testing is_manifold (single edge)..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    complex.add_edge(v0, v1);

    assert(TopologyOperations::is_manifold(complex) == true);

    std::cout << "  PASSED" << std::endl;
}

void test_is_manifold_two_triangles_sharing_edge() {
    std::cout << "Testing is_manifold (two triangles sharing edge)..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    VertexID v3 = complex.add_vertex();

    complex.add_triangle(v0, v1, v2);
    complex.add_triangle(v0, v1, v3);

    assert(TopologyOperations::is_manifold(complex) == true);

    std::cout << "  PASSED" << std::endl;
}

void test_is_manifold_three_triangles_sharing_edge() {
    std::cout << "Testing is_manifold (three triangles sharing edge)..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    VertexID v3 = complex.add_vertex();
    VertexID v4 = complex.add_vertex();

    complex.add_triangle(v0, v1, v2);
    complex.add_triangle(v0, v1, v3);
    complex.add_triangle(v0, v1, v4);

    // Edge (v0, v1) is shared by 3 triangles, not a manifold
    assert(TopologyOperations::is_manifold(complex) == false);

    std::cout << "  PASSED" << std::endl;
}

void test_is_manifold_complex_structure() {
    std::cout << "Testing is_manifold (complex structure)..." << std::endl;

    SimplicialComplex complex;

    // Create a tetrahedron (a 3D manifold boundary)
    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    VertexID v3 = complex.add_vertex();

    complex.add_triangle(v0, v1, v2);
    complex.add_triangle(v0, v1, v3);
    complex.add_triangle(v0, v2, v3);
    complex.add_triangle(v1, v2, v3);

    assert(TopologyOperations::is_manifold(complex) == true);

    std::cout << "  PASSED" << std::endl;
}

int main() {
    std::cout << "=== Advanced Topology Operations Tests ===" << std::endl;
    std::cout << std::endl;

    test_glue_simplices_by_boundary_basic();
    test_glue_simplices_by_boundary_invalid_simplex();
    test_glue_simplices_by_boundary_same_simplex();
    test_batch_glue_vertices_basic();
    test_batch_glue_vertices_invalid_vertex();
    test_compute_euler_characteristic_point();
    test_compute_euler_characteristic_edge();
    test_compute_euler_characteristic_triangle();
    test_compute_euler_characteristic_two_triangles();
    test_is_manifold_only_vertices();
    test_is_manifold_single_edge();
    test_is_manifold_two_triangles_sharing_edge();
    test_is_manifold_three_triangles_sharing_edge();
    test_is_manifold_complex_structure();

    std::cout << std::endl;
    std::cout << "=== All tests passed! ===" << std::endl;

    return 0;
}
