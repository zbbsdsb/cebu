#include "cebu/simplicial_complex.h"
#include <iostream>
#include <cassert>
#include <vector>

using namespace cebu;

void test_basic_construction() {
    std::cout << "Testing basic construction..." << std::endl;

    SimplicialComplex complex;

    // Test adding vertices
    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    VertexID v3 = complex.add_vertex();

    assert(v0 == 0);
    assert(v1 == 1);
    assert(v2 == 2);
    assert(v3 == 3);

    assert(complex.vertex_count() == 4);

    std::cout << "  OK: Vertex addition works" << std::endl;
}

void test_edge_creation() {
    std::cout << "Testing edge creation..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();

    // Add edges
    SimplexID e01 = complex.add_edge(v0, v1);
    SimplexID e12 = complex.add_edge(v1, v2);
    SimplexID e02 = complex.add_edge(v0, v2);

    assert(complex.has_simplex(e01));
    assert(complex.has_simplex(e12));
    assert(complex.has_simplex(e02));

    std::cout << "  OK: Edge creation works" << std::endl;
}

void test_triangle_creation() {
    std::cout << "Testing triangle creation..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();

    // Add triangle
    SimplexID tri = complex.add_triangle(v0, v1, v2);

    assert(complex.has_simplex(tri));

    const Simplex& triangle = complex.get_simplex(tri);
    assert(triangle.dimension() == 2);
    assert(triangle.vertices().size() == 3);

    std::cout << "  OK: Triangle creation works" << std::endl;
}

void test_vertex_to_simplex_mapping() {
    std::cout << "Testing vertex to simplex mapping..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();

    SimplexID e01 = complex.add_edge(v0, v1);
    SimplexID tri = complex.add_triangle(v0, v1, v2);

    // Find simplices containing v0
    auto simplices_with_v0 = complex.get_simplices_containing_vertex(v0);
    assert(simplices_with_v0.size() >= 2);  // Should contain e01 and tri

    std::cout << "  OK: Vertex to simplex mapping works" << std::endl;
}

void test_adjacency() {
    std::cout << "Testing adjacency..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    VertexID v3 = complex.add_vertex();

    SimplexID e01 = complex.add_edge(v0, v1);
    SimplexID e12 = complex.add_edge(v1, v2);
    SimplexID tri012 = complex.add_triangle(v0, v1, v2);
    SimplexID tri123 = complex.add_triangle(v1, v2, v3);

    // Edge e01 should be adjacent to triangle tri012 (sharing vertices)
    auto adjacent_to_e01 = complex.get_adjacent_simplices(e01);
    bool found_tri012 = std::find(adjacent_to_e01.begin(), adjacent_to_e01.end(),
                                   tri012) != adjacent_to_e01.end();
    assert(found_tri012);

    std::cout << "  OK: Adjacency works" << std::endl;
}

void test_facets() {
    std::cout << "Testing facets..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();

    // Add edges first (faces of the triangle)
    SimplexID e01 = complex.add_edge(v0, v1);
    SimplexID e12 = complex.add_edge(v1, v2);
    SimplexID e02 = complex.add_edge(v0, v2);

    // Add triangle
    SimplexID tri = complex.add_triangle(v0, v1, v2);

    // Get facets of the triangle
    auto facets = complex.get_facets(tri);

    // Should have three facets (three edges)
    assert(facets.size() == 3);

    // Check that all edges are in the facet list
    bool has_e01 = std::find(facets.begin(), facets.end(), e01) != facets.end();
    bool has_e12 = std::find(facets.begin(), facets.end(), e12) != facets.end();
    bool has_e02 = std::find(facets.begin(), facets.end(), e02) != facets.end();

    assert(has_e01 && has_e12 && has_e02);

    std::cout << "  OK: Facets work" << std::endl;
}

void test_dimension_query() {
    std::cout << "Testing dimension query..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();

    complex.add_edge(v0, v1);
    complex.add_edge(v1, v2);
    complex.add_triangle(v0, v1, v2);

    // Query 0-simplices (vertices)
    auto vertices = complex.get_simplices_of_dimension(0);
    assert(vertices.size() == 3);

    // Query 1-simplices (edges)
    auto edges = complex.get_simplices_of_dimension(1);
    assert(edges.size() == 2);

    // Query 2-simplices (triangles)
    auto triangles = complex.get_simplices_of_dimension(2);
    assert(triangles.size() == 1);

    std::cout << "  OK: Dimension query works" << std::endl;
}

void test_duplicate_prevention() {
    std::cout << "Testing duplicate prevention..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();

    // Add the same edge twice
    SimplexID e1 = complex.add_edge(v0, v1);
    SimplexID e2 = complex.add_edge(v1, v0);  // Different vertex order, same edge

    assert(e1 == e2);  // Should return the same ID

    std::cout << "  OK: Duplicate prevention works" << std::endl;
}

void tetrahedron_test() {
    std::cout << "Testing tetrahedron (3-simplex)..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    VertexID v3 = complex.add_vertex();

    // Add tetrahedron
    SimplexID tetra = complex.add_simplex({v0, v1, v2, v3});

    const Simplex& simplex = complex.get_simplex(tetra);
    assert(simplex.dimension() == 3);
    assert(simplex.vertices().size() == 4);

    std::cout << "  OK: Tetrahedron creation works" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Running cebu basic tests" << std::endl;
    std::cout << "========================================" << std::endl;

    try {
        test_basic_construction();
        test_edge_creation();
        test_triangle_creation();
        test_vertex_to_simplex_mapping();
        test_adjacency();
        test_facets();
        test_dimension_query();
        test_duplicate_prevention();
        tetrahedron_test();

        std::cout << "========================================" << std::endl;
        std::cout << "All tests passed!" << std::endl;
        std::cout << "========================================" << std::endl;

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
