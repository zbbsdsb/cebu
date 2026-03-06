#include "cebu/vertex_geometry.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace cebu {

// Point3D methods implementation
Point3D Point3D::operator*(float scalar) const {
    return Point3D(x * scalar, y * scalar, z * scalar);
}

float Point3D::dot(const Point3D& other) const {
    return x * other.x + y * other.y + z * other.z;
}

float Point3D::squared_distance_to(const Point3D& other) const {
    float dx = x - other.x;
    float dy = y - other.y;
    float dz = z - other.z;
    return dx * dx + dy * dy + dz * dz;
}

float Point3D::distance_to(const Point3D& other) const {
    return std::sqrt(squared_distance_to(other));
}

// BoundingBox methods implementation
void BoundingBox::expand(const Point3D& point) {
    min.x = std::min(min.x, point.x);
    min.y = std::min(min.y, point.y);
    min.z = std::min(min.z, point.z);
    max.x = std::max(max.x, point.x);
    max.y = std::max(max.y, point.y);
    max.z = std::max(max.z, point.z);
}

void BoundingBox::expand(const BoundingBox& other) {
    expand(other.min);
    expand(other.max);
}

bool BoundingBox::contains(const Point3D& point) const {
    return point.x >= min.x && point.x <= max.x &&
           point.y >= min.y && point.y <= max.y &&
           point.z >= min.z && point.z <= max.z;
}

bool BoundingBox::intersects(const BoundingBox& other) const {
    return min.x <= other.max.x && max.x >= other.min.x &&
           min.y <= other.max.y && max.y >= other.min.y &&
           min.z <= other.max.z && max.z >= other.min.z;
}

Point3D BoundingBox::center() const {
    return Point3D((min.x + max.x) * 0.5f,
                   (min.y + max.y) * 0.5f,
                   (min.z + max.z) * 0.5f);
}

float BoundingBox::surface_area() const {
    float dx = max.x - min.x;
    float dy = max.y - min.y;
    float dz = max.z - min.z;
    return 2.0f * (dx * dy + dy * dz + dz * dx);
}

float BoundingBox::volume() const {
    float dx = max.x - min.x;
    float dy = max.y - min.y;
    float dz = max.z - min.z;
    return dx * dy * dz;
}

float BoundingBox::extent(int axis) const {
    switch (axis) {
        case 0: return max.x - min.x;
        case 1: return max.y - min.y;
        case 2: return max.z - min.z;
        default: return 0.0f;
    }
}

BoundingBox BoundingBox::invalid() {
    return BoundingBox();
}

bool BoundingBox::is_valid() const {
    return min.x <= max.x && min.y <= max.y && min.z <= max.z;
}

// VertexGeometry implementation
VertexGeometry::VertexGeometry() {}

void VertexGeometry::set_position(VertexID vertex_id, const Point3D& position) {
    positions_[vertex_id] = position;
}

void VertexGeometry::set_position(VertexID vertex_id, float x, float y, float z) {
    positions_[vertex_id] = Point3D(x, y, z);
}

std::optional<Point3D> VertexGeometry::get_position(VertexID vertex_id) const {
    auto it = positions_.find(vertex_id);
    if (it != positions_.end()) {
        return it->second;
    }
    return std::nullopt;
}

bool VertexGeometry::has_position(VertexID vertex_id) const {
    return positions_.find(vertex_id) != positions_.end();
}

void VertexGeometry::remove_position(VertexID vertex_id) {
    positions_.erase(vertex_id);
}

BoundingBox VertexGeometry::compute_bounding_box() const {
    BoundingBox bbox;
    for (const auto& [vertex_id, position] : positions_) {
        bbox.expand(position);
    }
    return bbox;
}

void VertexGeometry::clear() {
    positions_.clear();
}

std::vector<VertexID> VertexGeometry::get_vertex_ids() const {
    std::vector<VertexID> ids;
    ids.reserve(positions_.size());
    for (const auto& [vertex_id, _] : positions_) {
        ids.push_back(vertex_id);
    }
    return ids;
}

} // namespace cebu
