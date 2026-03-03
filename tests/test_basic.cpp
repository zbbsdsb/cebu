#include "cebu/simplicial_complex.h"
#include <iostream>
#include <cassert>
#include <vector>

using namespace cebu;

void test_basic_construction() {
    std::cout << "Testing basic construction..." << std::endl;

    SimplicialComplex complex;

    // 测试添加顶点
    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    VertexID v3 = complex.add_vertex();

    assert(v0 == 0);
    assert(v1 == 1);
    assert(v2 == 2);
    assert(v3 == 3);

    assert(complex.vertex_count() == 4);

    std::cout << "  ✓ Vertex addition works" << std::endl;
}

void test_edge_creation() {
    std::cout << "Testing edge creation..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();

    // 添加边
    SimplexID e01 = complex.add_edge(v0, v1);
    SimplexID e12 = complex.add_edge(v1, v2);
    SimplexID e02 = complex.add_edge(v0, v2);

    assert(complex.has_simplex(e01));
    assert(complex.has_simplex(e12));
    assert(complex.has_simplex(e02));

    std::cout << "  ✓ Edge creation works" << std::endl;
}

void test_triangle_creation() {
    std::cout << "Testing triangle creation..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();

    // 添加三角形
    SimplexID tri = complex.add_triangle(v0, v1, v2);

    assert(complex.has_simplex(tri));

    const Simplex& triangle = complex.get_simplex(tri);
    assert(triangle.dimension() == 2);
    assert(triangle.vertices().size() == 3);

    std::cout << "  ✓ Triangle creation works" << std::endl;
}

void test_vertex_to_simplex_mapping() {
    std::cout << "Testing vertex to simplex mapping..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();

    SimplexID e01 = complex.add_edge(v0, v1);
    SimplexID tri = complex.add_triangle(v0, v1, v2);

    // 查找包含 v0 的单纯形
    auto simplices_with_v0 = complex.get_simplices_containing_vertex(v0);
    assert(simplices_with_v0.size() >= 2);  // 应该包含 e01 和 tri

    std::cout << "  ✓ Vertex to simplex mapping works" << std::endl;
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

    // 边 e01 应该与 tri012 相邻（共享顶点）
    auto adjacent_to_e01 = complex.get_adjacent_simplices(e01);
    bool found_tri012 = std::find(adjacent_to_e01.begin(), adjacent_to_e01.end(),
                                   tri012) != adjacent_to_e01.end();
    assert(found_tri012);

    std::cout << "  ✓ Adjacency works" << std::endl;
}

void test_facets() {
    std::cout << "Testing facets..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();

    // 先添加边（三角形的面）
    SimplexID e01 = complex.add_edge(v0, v1);
    SimplexID e12 = complex.add_edge(v1, v2);
    SimplexID e02 = complex.add_edge(v0, v2);

    // 添加三角形
    SimplexID tri = complex.add_triangle(v0, v1, v2);

    // 获取三角形的面
    auto facets = complex.get_facets(tri);

    // 应该有三个面（三条边）
    assert(facets.size() == 3);

    // 检查所有边都在面列表中
    bool has_e01 = std::find(facets.begin(), facets.end(), e01) != facets.end();
    bool has_e12 = std::find(facets.begin(), facets.end(), e12) != facets.end();
    bool has_e02 = std::find(facets.begin(), facets.end(), e02) != facets.end();

    assert(has_e01 && has_e12 && has_e02);

    std::cout << "  ✓ Facets work" << std::endl;
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

    // 查询 0-单纯形（顶点）
    auto vertices = complex.get_simplices_of_dimension(0);
    assert(vertices.size() == 3);

    // 查询 1-单纯形（边）
    auto edges = complex.get_simplices_of_dimension(1);
    assert(edges.size() == 2);

    // 查询 2-单纯形（三角形）
    auto triangles = complex.get_simplices_of_dimension(2);
    assert(triangles.size() == 1);

    std::cout << "  ✓ Dimension query works" << std::endl;
}

void test_duplicate_prevention() {
    std::cout << "Testing duplicate prevention..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();

    // 添加相同的边两次
    SimplexID e1 = complex.add_edge(v0, v1);
    SimplexID e2 = complex.add_edge(v1, v0);  // 顶点顺序不同，但边相同

    assert(e1 == e2);  // 应该返回相同的 ID

    std::cout << "  ✓ Duplicate prevention works" << std::endl;
}

void tetrahedron_test() {
    std::cout << "Testing tetrahedron (3-simplex)..." << std::endl;

    SimplicialComplex complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    VertexID v3 = complex.add_vertex();

    // 添加四面体
    SimplexID tetra = complex.add_simplex({v0, v1, v2, v3});

    const Simplex& simplex = complex.get_simplex(tetra);
    assert(simplex.dimension() == 3);
    assert(simplex.vertices().size() == 4);

    std::cout << "  ✓ Tetrahedron creation works" << std::endl;
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
        std::cout << "All tests passed! ✓" << std::endl;
        std::cout << "========================================" << std::endl;

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
