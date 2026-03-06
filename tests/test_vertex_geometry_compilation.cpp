// Minimal compilation test for VertexGeometry
#include "cebu/vertex_geometry.h"
#include <iostream>

int main() {
    std::cout << "Testing VertexGeometry compilation..." << std::endl;
    
    // Test Point3D
    cebu::Point3D p1(1.0f, 2.0f, 3.0f);
    cebu::Point3D p2(4.0f, 5.0f, 6.0f);
    cebu::Point3D p3 = p1 + p2;
    
    std::cout << "Point operations work!" << std::endl;
    
    // Test BoundingBox
    cebu::BoundingBox bbox(p1, p2);
    std::cout << "BoundingBox created!" << std::endl;
    
    // Test VertexGeometry
    cebu::VertexGeometry vg;
    vg.set_vertex(0, cebu::Point3D(0.0f, 0.0f, 0.0f));
    vg.set_vertex(1, cebu::Point3D(1.0f, 0.0f, 0.0f));
    vg.set_vertex(2, cebu::Point3D(0.0f, 1.0f, 0.0f));
    
    auto bbox_for_triangle = vg.compute_bounding_box({0, 1, 2});
    std::cout << "Triangle bounding box computed!" << std::endl;
    
    std::cout << "All basic compilation tests passed!" << std::endl;
    return 0;
}
