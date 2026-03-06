#include "cebu/vertex_geometry.h"
#include <iostream>

using namespace cebu;

int main() {
    // 测试 Point3D
    Point3D p1(1.0f, 2.0f, 3.0f);
    Point3D p2(4.0f, 5.0f, 6.0f);

    // 测试 BoundingBox
    BoundingBox bbox(p1, p2);
    Point3D center = bbox.center();

    // 测试 VertexGeometry
    VertexGeometry geometry;
    geometry.set_position(0, 1.0f, 2.0f, 3.0f);
    auto pos = geometry.get_position(0);

    std::cout << "Minimal BVH test passed!" << std::endl;
    std::cout << "Center: (" << center.x << ", " << center.y << ", " << center.z << ")" << std::endl;

    if (pos) {
        std::cout << "Position: (" << pos->x << ", " << pos->y << ", " << pos->z << ")" << std::endl;
    }

    return 0;
}
