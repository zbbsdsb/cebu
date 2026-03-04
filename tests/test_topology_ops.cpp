#include "cebu/topology_operations.h"
#include <iostream>
#include <cassert>

using namespace cebu;

void test_glue_vertices_basic() {
    std::cout << "Testing glue vertices basic..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();

    // Create edges
    SimplexID e01 = complex.add_edge(v0, v1);
    SimplexID e12 = complex.add_edge(v1, v2);

    // Glue v0 into v1
    VertexID result = TopologyOperations::glue_vertices(complex, v0, v1);

    assert(result == v1);
    assert(!complex.has_simplex(v0));  // v0 no longer exists
    assert(complex.vertex_count() == 2);

    // Edge e01 should now be between v1 and v1 (degenerate edge)
    // Edge e12 should still be between v1 and v2

    std::cout << "  OK: Glue vertices basic works" << std::endl;
}

void test_glue_vertices_self() {
    std::cout << "Testing glue vertices self..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();

    // Gluing vertex to itself should throw
    bool threw = false;
    try {
        TopologyOperations::glue_vertices(complex, v0, v0);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw);

    std::cout << "  OK: Glue vertices self check works" << std::endl;
}

void test_glue_vertices_nonexistent() {
    std::cout << "Testing glue vertices nonexistent..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();

    // Gluing with non-existent vertex should throw
    bool threw = false;
    try {
        TopologyOperations::glue_vertices(complex, v0, 999);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw);

    threw = false;
    try {
        TopologyOperations::glue_vertices(complex, 999, v0);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw);

    std::cout << "  OK: Glue vertices nonexistent check works" << std::endl;
}

void test_compute_boundary_single_vertex() {
    std::cout << "Testing compute boundary single vertex..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();

    auto boundary = TopologyOperations::compute_boundary(complex);

    // A single vertex is its own boundary
    assert(boundary.size() == 1);
    assert(boundary[0] == v0);

    std::cout << "  OK: Compute boundary single vertex works" << std::endl;
}

void test_compute_boundary_edge() {
    std::cout << "Testing compute boundary edge..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    SimplexID edge = complex.add_edge(v0, v1);

    auto boundary = TopologyOperations::compute_boundary(complex);

    // Edge is not a face of any higher-dimensional simplex, so it's on boundary
    // Vertices are faces of the edge, so they are NOT on boundary
    assert(boundary.size() == 1);
    assert(boundary[0] == edge);
    assert(std::find(boundary.begin(), boundary.end(), v0) == boundary.end());
    assert(std::find(boundary.begin(), boundary.end(), v1) == boundary.end());

    std::cout << "  OK: Compute boundary edge works" << std::endl;
}

void test_compute_boundary_triangle() {
    std::cout << "Testing compute boundary triangle..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();

    SimplexID tri = complex.add_triangle(v0, v1, v2);

    auto boundary = TopologyOperations::compute_boundary(complex);

    // Triangle is not a face of any higher-dimensional simplex, so it's on boundary
    // Vertices and edges are faces of triangle, so they are NOT on boundary
    assert(boundary.size() == 1);
    assert(boundary[0] == tri);
    assert(std::find(boundary.begin(), boundary.end(), v0) == boundary.end());
    assert(std::find(boundary.begin(), boundary.end(), v1) == boundary.end());
    assert(std::find(boundary.begin(), boundary.end(), v2) == boundary.end());

    std::cout << "  OK: Compute boundary triangle works" << std::endl;
}

void test_get_simplices_of_dimension() {
    std::cout << "Testing get simplices of dimension..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    VertexID v3 = complex.add_vertex();

    complex.add_edge(v0, v1);
    complex.add_edge(v1, v2);
    complex.add_triangle(v0, v1, v2);
    complex.add_edge(v2, v3);

    // Get vertices (0-simplices)
    auto dim0 = TopologyOperations::get_simplices_of_dimension(complex, 0);
    assert(dim0.size() == 4);

    // Get edges (1-simplices)
    auto dim1 = TopologyOperations::get_simplices_of_dimension(complex, 1);
    assert(dim1.size() == 3);

    // Get triangles (2-simplices)
    auto dim2 = TopologyOperations::get_simplices_of_dimension(complex, 2);
    assert(dim2.size() == 1);

    // No 3-simplices
    auto dim3 = TopologyOperations::get_simplices_of_dimension(complex, 3);
    assert(dim3.size() == 0);

    std::cout << "  OK: Get simplices of dimension works" << std::endl;
}

void test_compute_connected_components_single() {
    std::cout << "Testing compute connected components single..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();

    complex.add_edge(v0, v1);
    complex.add_edge(v1, v2);

    auto components = TopologyOperations::compute_connected_components(complex);

    // All connected, should be one component
    assert(components.size() == 1);
    assert(components[0].size() == 5);  // 3 vertices + 2 edges

    std::cout << "  OK: Compute connected components single works" << std::endl;
}

void test_compute_connected_components_multiple() {
    std::cout << "Testing compute connected components multiple..." << std::endl;

    SimplicialComplex complex;

    // First component
    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    complex.add_edge(v0, v1);

    // Second component (disconnected)
    VertexID v2 = complex.add_vertex();
    VertexID v3 = complex.add_vertex();
    complex.add_edge(v2, v3);

    // Third component (isolated vertex)
    VertexID v4 = complex.add_vertex();

    auto components = TopologyOperations::compute_connected_components(complex);

    assert(components.size() == 3);

    // Count total simplices
    size_t total = 0;
    for (const auto& comp : components) {
        total += comp.size();
    }
    assert(total == 5);  // 5 vertices + 2 edges

    std::cout << "  OK: Compute connected components multiple works" << std::endl;
}

int main() {
    std::cout << "=== Topology Operations Tests ===" << std::endl;
    std::cout << std::endl;

    test_glue_vertices_basic();
    test_glue_vertices_self();
    test_glue_vertices_nonexistent();
    test_compute_boundary_single_vertex();
    test_compute_boundary_edge();
    test_compute_boundary_triangle();
    test_get_simplices_of_dimension();
    test_compute_connected_components_single();
    test_compute_connected_components_multiple();

    std::cout << std::endl;
    std::cout << "=== All tests passed! ===" << std::endl;

    return 0;
}
