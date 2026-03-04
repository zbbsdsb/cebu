#include "cebu/simplicial_complex_labeled.h"
#include "cebu/absurdity.h"
#include <iostream>
#include <cassert>
#include <vector>

using namespace cebu;

void test_basic_labels() {
    std::cout << "Testing basic label operations..." << std::endl;

    SimplicialComplexDouble complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();

    SimplexID tri = complex.add_triangle(v0, v1, v2);

    // Set labels
    std::cout << "  v0=" << v0 << ", v1=" << v1 << ", v2=" << v2 << ", tri=" << tri << std::endl;
    complex.set_label(v0, 0.1);
    std::cout << "  Set label for v0: 0.1" << std::endl;
    complex.set_label(v1, 0.5);
    complex.set_label(v2, 0.9);
    complex.set_label(tri, 0.7);

    // Get labels
    auto label_v0 = complex.get_label(v0);
    assert(label_v0.has_value());
    std::cout << "  Expected: 0.1, Got: " << *label_v0 << std::endl;
    assert(*label_v0 == 0.1);

    auto label_tri = complex.get_label(tri);
    assert(label_tri.has_value());
    assert(*label_tri == 0.7);

    std::cout << "  OK: Basic label operations work" << std::endl;
}

void test_label_queries() {
    std::cout << "Testing label queries..." << std::endl;

    SimplicialComplexDouble complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    VertexID v3 = complex.add_vertex();

    complex.set_label(v0, 0.1);
    complex.set_label(v1, 0.3);
    complex.set_label(v2, 0.7);
    complex.set_label(v3, 0.9);

    // Find high labels
    auto high_labels = complex.find_high_labels(0.6);
    assert(high_labels.size() == 2);
    assert(std::find(high_labels.begin(), high_labels.end(), v2) != high_labels.end());
    assert(std::find(high_labels.begin(), high_labels.end(), v3) != high_labels.end());

    // Find low labels
    auto low_labels = complex.find_low_labels(0.5);
    assert(low_labels.size() == 2);
    assert(std::find(low_labels.begin(), low_labels.end(), v0) != low_labels.end());
    assert(std::find(low_labels.begin(), low_labels.end(), v1) != low_labels.end());

    // Find in range
    auto range_labels = complex.find_labels_in_range(0.2, 0.8);
    assert(range_labels.size() == 2);
    assert(std::find(range_labels.begin(), range_labels.end(), v1) != range_labels.end());
    assert(std::find(range_labels.begin(), range_labels.end(), v2) != range_labels.end());

    std::cout << "  OK: Label queries work" << std::endl;
}

void test_predicate_queries() {
    std::cout << "Testing predicate-based queries..." << std::endl;

    SimplicialComplexDouble complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();

    complex.set_label(v0, 0.25);
    complex.set_label(v1, 0.5);
    complex.set_label(v2, 0.75);

    // Find labels exactly 0.5
    auto exact = complex.find_by_label([](double label) {
        return std::abs(label - 0.5) < 1e-6;
    });
    assert(exact.size() == 1);
    assert(exact[0] == v1);

    // Find labels with specific property
    auto even_quarter = complex.find_by_label([](double label) {
        double x = label * 4;
        return std::abs(x - std::round(x)) < 1e-6;
    });
    assert(even_quarter.size() == 3);

    std::cout << "  OK: Predicate-based queries work" << std::endl;
}

void test_label_removal() {
    std::cout << "Testing label removal..." << std::endl;

    SimplicialComplexDouble complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();

    complex.set_label(v0, 0.5);
    complex.set_label(v1, 0.7);

    assert(complex.has_label(v0));
    assert(complex.labeled_count() == 2);

    complex.remove_label(v0);
    assert(!complex.has_label(v0));
    assert(complex.labeled_count() == 1);

    // Remove non-existent label
    bool removed = complex.remove_label(v0);
    assert(!removed);

    std::cout << "  OK: Label removal works" << std::endl;
}

void test_simplex_removal_with_labels() {
    std::cout << "Testing simplex removal with labels..." << std::endl;

    SimplicialComplexDouble complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();

    SimplexID tri = complex.add_triangle(v0, v1, v2);

    complex.set_label(tri, 0.8);

    assert(complex.has_label(tri));
    assert(complex.labeled_count() == 1);

    // Remove simplex (should also remove label)
    complex.remove_simplex(tri, false);

    assert(!complex.has_simplex(tri));
    assert(!complex.has_label(tri));
    assert(complex.labeled_count() == 0);

    std::cout << "  OK: Simplex removal with labels works" << std::endl;
}

void test_clear_labels() {
    std::cout << "Testing clear labels..." << std::endl;

    SimplicialComplexDouble complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();

    complex.set_label(v0, 0.1);
    complex.set_label(v1, 0.5);
    complex.set_label(v2, 0.9);

    assert(complex.labeled_count() == 3);

    complex.clear_labels();

    assert(complex.labeled_count() == 0);
    assert(!complex.has_label(v0));
    assert(!complex.has_label(v1));
    assert(!complex.has_label(v2));

    std::cout << "  OK: Clear labels works" << std::endl;
}

void test_absurdity_labels() {
    std::cout << "Testing absurdity labels..." << std::endl;

    SimplicialComplexAbsurdity complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();

    // Set absurdity labels
    Absurdity a1(0.6, 0.8, 0.9);  // High absurdity, high confidence
    Absurdity a2(0.1, 0.3, 0.5);  // Low absurdity, low confidence
    Absurdity a3(0.7, 0.9, 0.8);  // High absurdity, high confidence

    complex.set_label(v0, a1);
    complex.set_label(v1, a2);
    complex.set_label(v2, a3);

    // Get labels
    auto label_v0 = complex.get_label(v0);
    assert(label_v0.has_value());
    assert(label_v0->midpoint() == 0.7);
    assert(label_v0->confidence() == 0.9);

    // Find high absurdity
    auto high = complex.label_system().find_high_absurdity(0.6);
    std::cout << "  High absurdity count: " << high.size() << " (expected 2)" << std::endl;
    assert(high.size() == 2);

    // Find low absurdity
    auto low = complex.label_system().find_low_absurdity(0.4);
    assert(low.size() == 1);

    // Find high uncertainty
    auto uncertain = complex.label_system().find_high_uncertainty(0.2);
    assert(uncertain.size() >= 1);

    std::cout << "  OK: Absurdity labels work" << std::endl;
}

void test_absurdity_update() {
    std::cout << "Testing absurdity update..." << std::endl;

    SimplicialComplexAbsurdity complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();

    Absurdity a1(0.5, 0.5, 0.8);
    complex.set_label(v0, a1);

    AbsurdityContext ctx(0.9, 0.8, 0.0, 0.5, 0.1);

    auto label_before = complex.get_label(v0);
    complex.label_system().update_all(ctx);
    auto label_after = complex.get_label(v0);

    // Label should change after update
    assert(label_after.has_value());
    double delta = std::abs(label_after->midpoint() - label_before->midpoint());
    assert(delta > 1e-6);

    std::cout << "  OK: Absurdity update works" << std::endl;
}

void test_partial_labeling() {
    std::cout << "Testing partial labeling..." << std::endl;

    SimplicialComplexDouble complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    VertexID v3 = complex.add_vertex();

    // Only label some simplices
    complex.set_label(v0, 0.2);
    complex.set_label(v2, 0.8);

    assert(complex.has_label(v0));
    assert(!complex.has_label(v1));
    assert(complex.has_label(v2));
    assert(!complex.has_label(v3));

    assert(complex.labeled_count() == 2);
    assert(complex.simplex_count() == 4);

    // Query should only return labeled simplices
    auto all_labels = complex.find_by_label([](double) { return true; });
    assert(all_labels.size() == 2);

    std::cout << "  OK: Partial labeling works" << std::endl;
}

void test_combined_queries() {
    std::cout << "Testing combined queries (labels + topology)..." << std::endl;

    SimplicialComplexDouble complex;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    VertexID v3 = complex.add_vertex();

    SimplexID e01 = complex.add_edge(v0, v1);
    SimplexID e12 = complex.add_edge(v1, v2);
    SimplexID tri012 = complex.add_triangle(v0, v1, v2);

    complex.set_label(v0, 0.1);
    complex.set_label(v1, 0.9);
    complex.set_label(v2, 0.5);
    complex.set_label(tri012, 0.7);

    // Find high-labeled vertices
    auto high_labeled = complex.find_high_labels(0.6);
    assert(high_labeled.size() == 2);  // v1 and tri012

    // Check if high-labeled simplices are adjacent
    for (SimplexID id : high_labeled) {
        auto adjacent = complex.get_adjacent_simplices(id);
        // Adjacency relationships should be maintained
    }

    std::cout << "  OK: Combined queries work" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Running cebu label tests" << std::endl;
    std::cout << "========================================" << std::endl;

    try {
        test_basic_labels();
        test_label_queries();
        test_predicate_queries();
        test_label_removal();
        test_simplex_removal_with_labels();
        test_clear_labels();
        test_absurdity_labels();
        test_absurdity_update();
        test_partial_labeling();
        test_combined_queries();

        std::cout << "========================================" << std::endl;
        std::cout << "All label tests passed!" << std::endl;
        std::cout << "========================================" << std::endl;

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
