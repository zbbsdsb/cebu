#include "cebu/vertex_geometry.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace cebu {

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
