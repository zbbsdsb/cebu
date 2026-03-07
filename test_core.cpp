#include "cebu/simplicial_complex.h"
#include <iostream>

using namespace cebu;

int main() {
    // Create a simplicial complex
    SimplicialComplex complex;

    // Add vertices
    auto v0 = complex.add_vertex();
    auto v1 = complex.add_vertex();
    auto v2 = complex.add_vertex();

    std::cout << "Added vertices: " << v0 << ", " << v1 << ", " << v2 << std::endl;

    // Add edges
    auto e01 = complex.add_edge(v0, v1);
    auto e12 = complex.add_edge(v1, v2);
    auto e20 = complex.add_edge(v2, v0);

    std::cout << "Added edges: " << e01 << ", " << e12 << ", " << e20 << std::endl;

    // Add triangle
    auto triangle = complex.add_triangle(v0, v1, v2);

    std::cout << "Added triangle: " << triangle << std::endl;

    // Print statistics
    std::cout << "\nStatistics:" << std::endl;
    std::cout << "Vertices: " << complex.vertex_count() << std::endl;
    std::cout << "Edges: " << complex.simplex_count(1) << std::endl;
    std::cout << "Triangles: " << complex.simplex_count(2) << std::endl;

    return 0;
}