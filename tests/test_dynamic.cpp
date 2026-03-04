#include "cebu/simplicial_complex.h"
#include <iostream>
#include <cassert>
#include <vector>

using namespace cebu;

void test_simplex_removal() {
    std::cout << "Testing simplex removal..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();

    SimplexID e01 = complex.add_edge(v0, v1);
    SimplexID e12 = complex.add_edge(v1, v2);
    SimplexID tri = complex.add_triangle(v0, v1, v2);

    // Remove an edge
    bool removed = complex.remove_simplex(e01, false);
    assert(removed == true);
    assert(!complex.has_simplex(e01));

    std::cout << "  OK: Simplex removal works" << std::endl;
}

void test_vertex_removal_cascade() {
    std::cout << "Testing vertex removal with cascade..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();

    SimplexID e01 = complex.add_edge(v0, v1);
    SimplexID e12 = complex.add_edge(v1, v2);
    SimplexID tri = complex.add_triangle(v0, v1, v2);

    size_t count_before = complex.simplex_count();

    // Remove vertex with cascade
    bool removed = complex.remove_vertex(v1, true);
    assert(removed == true);

    // All simplices containing v1 should be removed
    assert(!complex.has_simplex(e01));
    assert(!complex.has_simplex(e12));
    assert(!complex.has_simplex(tri));

    std::cout << "  OK: Vertex removal with cascade works" << std::endl;
}

void test_cascade_removal() {
    std::cout << "Testing cascade removal..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    VertexID v3 = complex.add_vertex();

    // Add tetrahedron (3-simplex)
    SimplexID tetra = complex.add_simplex({v0, v1, v2, v3});

    // Add a face (triangle)
    SimplexID tri = complex.add_triangle(v0, v1, v2);

    // Remove triangle with cascade - should also remove tetrahedron
    bool removed = complex.remove_simplex(tri, true);
    assert(removed == true);
    assert(!complex.has_simplex(tri));
    assert(!complex.has_simplex(tetra));

    std::cout << "  OK: Cascade removal works" << std::endl;
}

void test_add_after_remove() {
    std::cout << "Testing add after remove..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();

    SimplexID e01 = complex.add_edge(v0, v1);

    // Remove edge
    complex.remove_simplex(e01, false);

    // Add edge again - should get a new ID
    SimplexID e01_new = complex.add_edge(v0, v1);
    assert(e01_new != e01);
    assert(complex.has_simplex(e01_new));

    std::cout << "  OK: Add after remove works" << std::endl;
}

void test_adjacency_after_removal() {
    std::cout << "Testing adjacency after removal..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();

    SimplexID e01 = complex.add_edge(v0, v1);
    SimplexID e12 = complex.add_edge(v1, v2);
    SimplexID tri = complex.add_triangle(v0, v1, v2);

    // e01 should be adjacent to tri
    auto adj_e01 = complex.get_adjacent_simplices(e01);
    assert(std::find(adj_e01.begin(), adj_e01.end(), tri) != adj_e01.end());

    // Remove e01
    complex.remove_simplex(e01, false);

    // tri should no longer be adjacent to e01
    auto adj_tri = complex.get_adjacent_simplices(tri);
    assert(std::find(adj_tri.begin(), adj_tri.end(), e01) == adj_tri.end());

    std::cout << "  OK: Adjacency after removal works" << std::endl;
}

void test_vertex_mapping_after_removal() {
    std::cout << "Testing vertex mapping after removal..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();

    SimplexID e01 = complex.add_edge(v0, v1);
    SimplexID e12 = complex.add_edge(v1, v2);

    // v1 should be in two edges
    auto simplices_with_v1 = complex.get_simplices_containing_vertex(v1);
    assert(simplices_with_v1.size() == 2);

    // Remove e01
    complex.remove_simplex(e01, false);

    // v1 should now be in only one edge
    simplices_with_v1 = complex.get_simplices_containing_vertex(v1);
    assert(simplices_with_v1.size() == 1);

    std::cout << "  OK: Vertex mapping after removal works" << std::endl;
}

void test_multiple_operations() {
    std::cout << "Testing multiple add/remove operations..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    VertexID v3 = complex.add_vertex();

    // Add multiple simplices
    SimplexID e01 = complex.add_edge(v0, v1);
    SimplexID e12 = complex.add_edge(v1, v2);
    SimplexID e23 = complex.add_edge(v2, v3);
    SimplexID tri012 = complex.add_triangle(v0, v1, v2);

    size_t count1 = complex.simplex_count();

    // Remove some
    complex.remove_simplex(e01, false);
    complex.remove_simplex(e23, false);

    size_t count2 = complex.simplex_count();
    assert(count2 == count1 - 2);

    // Add new
    SimplexID e03 = complex.add_edge(v0, v3);
    SimplexID tri123 = complex.add_triangle(v1, v2, v3);

    size_t count3 = complex.simplex_count();
    assert(count3 == count2 + 2);

    std::cout << "  OK: Multiple add/remove operations work" << std::endl;
}

void test_remove_nonexistent() {
    std::cout << "Testing removal of non-existent simplex..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();

    SimplexID e01 = complex.add_edge(v0, v1);

    // Try to remove non-existent simplex
    bool removed = complex.remove_simplex(999, false);
    assert(removed == false);

    // Try to remove non-existent vertex
    removed = complex.remove_vertex(999, true);
    assert(removed == false);

    std::cout << "  OK: Removal of non-existent works" << std::endl;
}

void test_remove_without_cascade() {
    std::cout << "Testing removal without cascade..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();

    SimplexID e01 = complex.add_edge(v0, v1);
    SimplexID e12 = complex.add_edge(v1, v2);
    SimplexID tri = complex.add_triangle(v0, v1, v2);

    // Remove vertex without cascade
    bool removed = complex.remove_vertex(v1, false);
    assert(removed == true);

    // Simplices should still exist
    assert(complex.has_simplex(e01));
    assert(complex.has_simplex(e12));
    assert(complex.has_simplex(tri));

    std::cout << "  OK: Removal without cascade works" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Running cebu dynamic tests" << std::endl;
    std::cout << "========================================" << std::endl;

    try {
        test_simplex_removal();
        test_vertex_removal_cascade();
        test_cascade_removal();
        test_add_after_remove();
        test_adjacency_after_removal();
        test_vertex_mapping_after_removal();
        test_multiple_operations();
        test_remove_nonexistent();
        test_remove_without_cascade();

        std::cout << "========================================" << std::endl;
        std::cout << "All dynamic tests passed!" << std::endl;
        std::cout << "========================================" << std::endl;

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
