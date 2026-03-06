// Minimal compilation test for BVH
#include "cebu/bvh.h"
#include "cebu/simplicial_complex.h"
#include <iostream>

int main() {
    std::cout << "Testing BVH compilation..." << std::endl;
    
    // Test Point3D
    cebu::Point3D p1(1.0f, 2.0f, 3.0f);
    cebu::Point3D p2(4.0f, 5.0f, 6.0f);
    
    float dist = p1.distance_to(p2);
    std::cout << "Distance between points: " << dist << std::endl;
    
    // Test BoundingBox
    cebu::BoundingBox bbox;
    bbox.expand(p1);
    bbox.expand(p2);
    
    std::cout << "BoundingBox: [" 
              << bbox.min.x << "," << bbox.min.y << "," << bbox.min.z << "] to ["
              << bbox.max.x << "," << bbox.max.y << "," << bbox.max.z << "]" << std::endl;
    
    // Test BVHTree
    cebu::BVHTree tree;
    std::cout << "BVHTree created successfully" << std::endl;
    
    std::cout << "All basic compilation tests passed!" << std::endl;
    return 0;
}
