#include "cebu/vertex_geometry.h"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace cebu;

void test_point3d_operations() {
    std::cout << "Testing Point3D operations..." << std::endl;

    Point3D p1(1.0f, 2.0f, 3.0f);
    Point3D p2(4.0f, 5.0f, 6.0f);

    // Addition
    Point3D p3 = p1 + p2;
    assert(std::abs(p3.x - 5.0f) < 1e-6f);
    assert(std::abs(p3.y - 7.0f) < 1e-6f);
    assert(std::abs(p3.z - 9.0f) < 1e-6f);

    // Subtraction
    Point3D p4 = p2 - p1;
    assert(std::abs(p4.x - 3.0f) < 1e-6f);
    assert(std::abs(p4.y - 3.0f) < 1e-6f);
    assert(std::abs(p4.z - 3.0f) < 1e-6f);

    // Scalar multiplication
    Point3D p5 = p1 * 2.0f;
    assert(std::abs(p5.x - 2.0f) < 1e-6f);
    assert(std::abs(p5.y - 4.0f) < 1e-6f);
    assert(std::abs(p5.z - 6.0f) < 1e-6f);

    // Dot product
    float dot = p1.dot(p2);
    assert(std::abs(dot - 32.0f) < 1e-6f);

    // Distance
    float dist = p1.distance_to(p2);
    assert(std::abs(dist - std::sqrt(27.0f)) < 1e-6f);

    // Squared distance
    float sqdist = p1.squared_distance_to(p2);
    assert(std::abs(sqdist - 27.0f) < 1e-6f);

    std::cout << "  Point3D operations: PASSED" << std::endl;
}

void test_bounding_box_operations() {
    std::cout << "Testing BoundingBox operations..." << std::endl;

    // Create a bounding box
    BoundingBox bbox1(Point3D(0.0f, 0.0f, 0.0f), Point3D(10.0f, 10.0f, 10.0f));

    // Test center
    Point3D center = bbox1.center();
    assert(std::abs(center.x - 5.0f) < 1e-6f);
    assert(std::abs(center.y - 5.0f) < 1e-6f);
    assert(std::abs(center.z - 5.0f) < 1e-6f);

    // Test surface area
    float area = bbox1.surface_area();
    assert(std::abs(area - 600.0f) < 1e-6f);

    // Test volume
    float volume = bbox1.volume();
    assert(std::abs(volume - 1000.0f) < 1e-6f);

    // Test extent
    assert(std::abs(bbox1.extent(0) - 10.0f) < 1e-6f);
    assert(std::abs(bbox1.extent(1) - 10.0f) < 1e-6f);
    assert(std::abs(bbox1.extent(2) - 10.0f) < 1e-6f);

    // Test contains
    assert(bbox1.contains(Point3D(5.0f, 5.0f, 5.0f)));
    assert(bbox1.contains(Point3D(0.0f, 0.0f, 0.0f)));
    assert(bbox1.contains(Point3D(10.0f, 10.0f, 10.0f)));
    assert(!bbox1.contains(Point3D(11.0f, 5.0f, 5.0f)));
    assert(!bbox1.contains(Point3D(-1.0f, 5.0f, 5.0f)));

    // Test intersects
    BoundingBox bbox2(Point3D(5.0f, 5.0f, 5.0f), Point3D(15.0f, 15.0f, 15.0f));
    assert(bbox1.intersects(bbox2));

    BoundingBox bbox3(Point3D(20.0f, 20.0f, 20.0f), Point3D(30.0f, 30.0f, 30.0f));
    assert(!bbox1.intersects(bbox3));

    // Test expand with point
    bbox1.expand(Point3D(15.0f, 15.0f, 15.0f));
    assert(std::abs(bbox1.max.x - 15.0f) < 1e-6f);

    // Test expand with bbox
    bbox1.expand(BoundingBox(Point3D(-5.0f, -5.0f, -5.0f), Point3D(0.0f, 0.0f, 0.0f)));
    assert(std::abs(bbox1.min.x + 5.0f) < 1e-6f);

    // Test validity
    BoundingBox invalid = BoundingBox::invalid();
    assert(!invalid.is_valid());
    assert(bbox1.is_valid());

    std::cout << "  BoundingBox operations: PASSED" << std::endl;
}

void test_vertex_geometry_basic() {
    std::cout << "Testing VertexGeometry basic operations..." << std::endl;

    VertexGeometry geometry;

    // Test set and get
    VertexID v1 = 1;
    geometry.set_position(v1, 1.0f, 2.0f, 3.0f);

    auto pos = geometry.get_position(v1);
    assert(pos.has_value());
    assert(std::abs(pos->x - 1.0f) < 1e-6f);
    assert(std::abs(pos->y - 2.0f) < 1e-6f);
    assert(std::abs(pos->z - 3.0f) < 1e-6f);

    // Test set with Point3D
    VertexID v2 = 2;
    geometry.set_position(v2, Point3D(4.0f, 5.0f, 6.0f));
    pos = geometry.get_position(v2);
    assert(pos.has_value());
    assert(std::abs(pos->x - 4.0f) < 1e-6f);

    // Test has_position
    assert(geometry.has_position(v1));
    assert(geometry.has_position(v2));
    assert(!geometry.has_position(999));

    // Test vertex count
    assert(geometry.vertex_count() == 2);

    // Test remove
    geometry.remove_position(v1);
    assert(!geometry.has_position(v1));
    assert(geometry.vertex_count() == 1);

    std::cout << "  VertexGeometry basic operations: PASSED" << std::endl;
}

void test_vertex_geometry_bounding_box() {
    std::cout << "Testing VertexGeometry bounding box..." << std::endl;

    VertexGeometry geometry;

    // Empty geometry should have invalid bbox
    BoundingBox empty_bbox = geometry.compute_bounding_box();
    assert(!empty_bbox.is_valid());

    // Add some vertices
    geometry.set_position(1, 0.0f, 0.0f, 0.0f);
    geometry.set_position(2, 10.0f, 10.0f, 10.0f);
    geometry.set_position(3, 5.0f, 5.0f, 5.0f);

    BoundingBox bbox = geometry.compute_bounding_box();
    assert(bbox.is_valid());
    assert(std::abs(bbox.min.x - 0.0f) < 1e-6f);
    assert(std::abs(bbox.max.x - 10.0f) < 1e-6f);

    std::cout << "  VertexGeometry bounding box: PASSED" << std::endl;
}

void test_vertex_geometry_clear() {
    std::cout << "Testing VertexGeometry clear..." << std::endl;

    VertexGeometry geometry;

    // Add some vertices
    geometry.set_position(1, 1.0f, 2.0f, 3.0f);
    geometry.set_position(2, 4.0f, 5.0f, 6.0f);
    assert(geometry.vertex_count() == 2);

    // Clear
    geometry.clear();
    assert(geometry.vertex_count() == 0);
    assert(!geometry.has_position(1));
    assert(!geometry.has_position(2));

    std::cout << "  VertexGeometry clear: PASSED" << std::endl;
}

void test_vertex_geometry_get_vertex_ids() {
    std::cout << "Testing VertexGeometry get_vertex_ids..." << std::endl;

    VertexGeometry geometry;

    // Add some vertices
    geometry.set_position(1, 1.0f, 2.0f, 3.0f);
    geometry.set_position(2, 4.0f, 5.0f, 6.0f);
    geometry.set_position(3, 7.0f, 8.0f, 9.0f);

    std::vector<VertexID> ids = geometry.get_vertex_ids();
    assert(ids.size() == 3);

    // Check that all IDs are present
    bool found1 = std::find(ids.begin(), ids.end(), 1) != ids.end();
    bool found2 = std::find(ids.begin(), ids.end(), 2) != ids.end();
    bool found3 = std::find(ids.begin(), ids.end(), 3) != ids.end();

    assert(found1);
    assert(found2);
    assert(found3);

    std::cout << "  VertexGeometry get_vertex_ids: PASSED" << std::endl;
}

void test_bounding_box_edge_cases() {
    std::cout << "Testing BoundingBox edge cases..." << std::endl;

    // Test degenerate box (single point)
    BoundingBox degenerate(Point3D(5.0f, 5.0f, 5.0f), Point3D(5.0f, 5.0f, 5.0f));
    assert(degenerate.is_valid());
    assert(degenerate.surface_area() == 0.0f);
    assert(degenerate.volume() == 0.0f);

    // Test box with zero extent on one axis
    BoundingBox flat_box(Point3D(0.0f, 0.0f, 0.0f), Point3D(10.0f, 10.0f, 0.0f));
    assert(flat_box.is_valid());
    assert(flat_box.volume() == 0.0f);
    assert(flat_box.surface_area() > 0.0f);

    // Test intersection with touching boxes
    BoundingBox box1(Point3D(0.0f, 0.0f, 0.0f), Point3D(10.0f, 10.0f, 10.0f));
    BoundingBox touching(Point3D(10.0f, 10.0f, 10.0f), Point3D(20.0f, 20.0f, 20.0f));
    assert(box1.intersects(touching));  // Touching at a point

    std::cout << "  BoundingBox edge cases: PASSED" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "      Vertex Geometry Test Suite       " << std::endl;
    std::cout << "========================================" << std::endl;

    try {
        test_point3d_operations();
        test_bounding_box_operations();
        test_vertex_geometry_basic();
        test_vertex_geometry_bounding_box();
        test_vertex_geometry_clear();
        test_vertex_geometry_get_vertex_ids();
        test_bounding_box_edge_cases();

        std::cout << "\n========================================" << std::endl;
        std::cout << "         ALL TESTS PASSED!              " << std::endl;
        std::cout << "========================================" << std::endl;

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
}
