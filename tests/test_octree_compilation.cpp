// Minimal compilation test for Octree
#include "cebu/octree.h"
#include <iostream>

int main() {
    std::cout << "Testing Octree compilation..." << std::endl;
    
    // Test OctreeNode
    cebu::OctreeNode node;
    std::cout << "OctreeNode created successfully" << std::endl;
    
    // Test Octree
    cebu::Octree tree;
    std::cout << "Octree created successfully" << std::endl;
    
    std::cout << "All basic compilation tests passed!" << std::endl;
    return 0;
}
