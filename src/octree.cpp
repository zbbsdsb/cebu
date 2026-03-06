#include "cebu/octree.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace cebu {

// Octree implementation
Octree::Octree(size_t max_depth, size_t max_simplices_per_node)
    : max_depth_(max_depth),
      max_simplices_per_node_(max_simplices_per_node),
      vertex_geometry_(nullptr),
      get_simplex_vertices_(nullptr) {
}

void Octree::build(
    const std::vector<SimplexID>& simplices,
    const VertexGeometry& vertex_geometry,
    std::function<std::vector<VertexID>(SimplexID)> get_simplex_vertices
) {
    // Clear existing tree
    clear();

    // Store references
    vertex_geometry_ = const_cast<VertexGeometry*>(&vertex_geometry);
    get_simplex_vertices_ = get_simplex_vertices;
    simplex_ids_ = simplices;

    if (simplices.empty()) {
        return;
    }

    // Compute bounding box for all simplices
    BoundingBox root_bbox = compute_bbox(simplices);

    // Build the tree
    build_recursive(simplices, root_bbox, 0);

    if (nodes_.empty()) {
        clear();
    }
}

void Octree::rebuild() {
    if (simplex_ids_.empty() || !vertex_geometry_ || !get_simplex_vertices_) {
        return;
    }

    // Store the current simplices
    std::vector<SimplexID> simplices = simplex_ids_;

    // Rebuild
    build(simplices, *vertex_geometry_, get_simplex_vertices_);
}

void Octree::add_simplex(SimplexID simplex_id) {
    // For now, use simple rebuild strategy
    simplex_ids_.push_back(simplex_id);

    // Rebuild if tree is small enough
    if (simplex_ids_.size() < max_simplices_per_node_ * 4) {
        rebuild();
    }
}

void Octree::remove_simplex(SimplexID simplex_id) {
    // Remove from simplex list
    auto it = std::find(simplex_ids_.begin(), simplex_ids_.end(), simplex_id);
    if (it != simplex_ids_.end()) {
        simplex_ids_.erase(it);
    }

    // Remove from mapping
    simplex_to_leaves_.erase(simplex_id);

    // Rebuild
    if (!simplex_ids_.empty()) {
        rebuild();
    } else {
        clear();
    }
}

void Octree::clear() {
    nodes_.clear();
    simplex_ids_.clear();
    simplex_to_leaves_.clear();
}

size_t Octree::create_node(const BoundingBox& bbox, int depth) {
    size_t idx = nodes_.size();
    nodes_.emplace_back();
    nodes_[idx].bbox = bbox;
    nodes_[idx].depth = depth;
    return idx;
}

size_t Octree::build_recursive(const std::vector<SimplexID>& simplex_ids,
                              const BoundingBox& bbox, int depth) {
    // Create a new node
    size_t node_idx = create_node(bbox, depth);
    OctreeNode& node = nodes_[node_idx];

    // Base case: create leaf node
    if (simplex_ids.size() <= max_simplices_per_node_ ||
        depth >= static_cast<int>(max_depth_)) {
        node.is_leaf = true;
        node.simplices = simplex_ids;

        // Update mapping
        for (SimplexID id : simplex_ids) {
            simplex_to_leaves_[id] = node_idx;
        }

        return node_idx;
    }

    // Subdivide into 8 octants
    subdivide_node(node_idx);

    // Distribute simplices to children
    for (SimplexID simplex_id : simplex_ids) {
        int octant = get_simplex_octant(simplex_id, bbox);

        if (octant >= 0 && octant < 8) {
            // Simplex fits in this octant
            size_t child_idx = nodes_[node_idx].get_child(octant);
            if (child_idx != SIZE_MAX) {
                OctreeNode& child = nodes_[child_idx];
                child.simplices.push_back(simplex_id);
            }
        } else {
            // Simplex spans multiple octants, keep in current node
            node.simplices.push_back(simplex_id);
        }
    }

    // Recursively build non-empty children
    for (int octant = 0; octant < 8; ++octant) {
        size_t child_idx = nodes_[node_idx].get_child(octant);
        if (child_idx != SIZE_MAX) {
            const OctreeNode& child = nodes_[child_idx];
            if (!child.simplices.empty()) {
                // Replace child with recursively built subtree
                BoundingBox child_bbox = get_octant_bbox(bbox, octant);
                size_t new_child = build_recursive(child.simplices, child_bbox, depth + 1);
                nodes_[node_idx].set_child(octant, new_child);
            } else {
                // Remove empty child
                nodes_[node_idx].set_child(octant, SIZE_MAX);
            }
        }
    }

    node.is_leaf = false;
    return node_idx;
}

void Octree::subdivide_node(size_t node_idx) {
    OctreeNode& node = nodes_[node_idx];
    Point3D center = node.bbox.center();

    // Create 8 children
    for (int octant = 0; octant < 8; ++octant) {
        BoundingBox child_bbox = get_octant_bbox(node.bbox, octant);
        size_t child_idx = create_node(child_bbox, node.depth + 1);
        nodes_[node_idx].set_child(octant, child_idx);
    }
}

BoundingBox Octree::compute_bbox(const std::vector<SimplexID>& simplex_ids) const {
    BoundingBox bbox;
    for (SimplexID id : simplex_ids) {
        std::vector<VertexID> vertices = get_simplex_vertices_(id);
        for (VertexID vid : vertices) {
            auto pos = vertex_geometry_->get_position(vid);
            if (pos) {
                bbox.expand(*pos);
            }
        }
    }
    return bbox;
}

Point3D Octree::get_simplex_centroid(SimplexID simplex_id) const {
    std::vector<VertexID> vertices = get_simplex_vertices_(simplex_id);
    if (vertices.empty()) {
        return Point3D();
    }

    Point3D centroid;
    for (VertexID vid : vertices) {
        auto pos = vertex_geometry_->get_position(vid);
        if (pos) {
            centroid = centroid + *pos;
        }
    }
    centroid = centroid * (1.0f / vertices.size());
    return centroid;
}

int Octree::get_simplex_octant(SimplexID simplex_id, const BoundingBox& node_bbox) const {
    Point3D center = node_bbox.center();

    std::vector<VertexID> vertices = get_simplex_vertices_(simplex_id);
    if (vertices.empty()) {
        return 0;
    }

    // Get first vertex's octant
    auto first_pos = vertex_geometry_->get_position(vertices[0]);
    if (!first_pos) {
        return 0;
    }

    int octant = OctreeNode::get_octant(*first_pos, center);

    // Check if all vertices are in the same octant
    for (size_t i = 1; i < vertices.size(); ++i) {
        auto pos = vertex_geometry_->get_position(vertices[i]);
        if (pos) {
            int vertex_octant = OctreeNode::get_octant(*pos, center);
            if (vertex_octant != octant) {
                return -1;  // Spans multiple octants
            }
        }
    }

    return octant;
}

bool Octree::simplex_fits_in_octant(SimplexID simplex_id, const BoundingBox& node_bbox, int octant) const {
    BoundingBox octant_bbox = get_octant_bbox(node_bbox, octant);

    std::vector<VertexID> vertices = get_simplex_vertices_(simplex_id);
    for (VertexID vid : vertices) {
        auto pos = vertex_geometry_->get_position(vid);
        if (pos && !octant_bbox.contains(*pos)) {
            return false;
        }
    }

    return true;
}

BoundingBox Octree::get_octant_bbox(const BoundingBox& node_bbox, int octant) const {
    Point3D center = node_bbox.center();

    float min_x, max_x;
    float min_y, max_y;
    float min_z, max_z;

    // Determine bounds based on octant
    if (octant & 1) {
        min_x = center.x;
        max_x = node_bbox.max.x;
    } else {
        min_x = node_bbox.min.x;
        max_x = center.x;
    }

    if (octant & 2) {
        min_y = center.y;
        max_y = node_bbox.max.y;
    } else {
        min_y = node_bbox.min.y;
        max_y = center.y;
    }

    if (octant & 4) {
        min_z = center.z;
        max_z = node_bbox.max.z;
    } else {
        min_z = node_bbox.min.z;
        max_z = center.z;
    }

    return BoundingBox(Point3D(min_x, min_y, min_z), Point3D(max_x, max_y, max_z));
}

std::vector<SimplexID> Octree::query_range(const BoundingBox& bbox) const {
    std::vector<SimplexID> results;
    if (!is_built() || nodes_.empty()) {
        return results;
    }
    query_range_recursive(0, bbox, results);
    return results;
}

void Octree::query_range_recursive(size_t node_idx, const BoundingBox& bbox,
                                 std::vector<SimplexID>& results) const {
    const OctreeNode& node = nodes_[node_idx];

    // Check if this node's bbox intersects with query bbox
    if (!node.bbox.intersects(bbox)) {
        return;
    }

    // Add simplices in this node
    for (SimplexID id : node.simplices) {
        results.push_back(id);
    }

    if (node.is_leaf) {
        return;
    }

    // Recursively query children
    for (int octant = 0; octant < 8; ++octant) {
        size_t child_idx = node.get_child(octant);
        if (child_idx != SIZE_MAX) {
            query_range_recursive(child_idx, bbox, results);
        }
    }
}

std::vector<SimplexID> Octree::query_point(const Point3D& point) const {
    std::vector<SimplexID> results;
    if (!is_built() || nodes_.empty()) {
        return results;
    }
    query_point_recursive(0, point, results);
    return results;
}

void Octree::query_point_recursive(size_t node_idx, const Point3D& point,
                                  std::vector<SimplexID>& results) const {
    const OctreeNode& node = nodes_[node_idx];

    // Check if point is in this node's bbox
    if (!node.bbox.contains(point)) {
        return;
    }

    // Add simplices in this node
    for (SimplexID id : node.simplices) {
        results.push_back(id);
    }

    if (node.is_leaf) {
        return;
    }

    // Determine which octant the point is in
    int octant = OctreeNode::get_octant(point, node.bbox.center());
    size_t child_idx = node.get_child(octant);

    if (child_idx != SIZE_MAX) {
        query_point_recursive(child_idx, point, results);
    }
}

std::vector<SimplexID> Octree::query_nearest(const Point3D& point, size_t k) const {
    std::vector<SimplexID> results;
    if (!is_built() || nodes_.empty() || k == 0) {
        return results;
    }

    std::vector<std::pair<float, SimplexID>> candidates;
    query_nearest_recursive(0, point, k, candidates);

    // Sort by distance and extract top k
    std::sort(candidates.begin(), candidates.end());
    size_t count = std::min(k, candidates.size());
    results.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        results.push_back(candidates[i].second);
    }

    return results;
}

void Octree::query_nearest_recursive(size_t node_idx, const Point3D& point,
                                   size_t k, std::vector<std::pair<float, SimplexID>>& candidates) const {
    const OctreeNode& node = nodes_[node_idx];

    if (node.is_leaf) {
        // Check each simplex in the leaf
        for (SimplexID id : node.simplices) {
            Point3D centroid = get_simplex_centroid(id);
            float dist = point.squared_distance_to(centroid);

            candidates.push_back({dist, id});
            if (candidates.size() > k * 2) {
                // Prune if we have too many candidates
                std::sort(candidates.begin(), candidates.end());
                candidates.resize(k);
            }
        }
    } else {
        // Visit children in order of proximity
        Point3D center = node.bbox.center();
        float node_center_dist = point.squared_distance_to(center);

        // Collect children with their distances
        std::vector<std::pair<float, size_t>> child_dist;
        for (int octant = 0; octant < 8; ++octant) {
            size_t child_idx = node.get_child(octant);
            if (child_idx != SIZE_MAX) {
                const OctreeNode& child = nodes_[child_idx];
                Point3D child_center = child.bbox.center();
                float dist = point.squared_distance_to(child_center);
                child_dist.push_back({dist, child_idx});
            }
        }

        // Sort by distance
        std::sort(child_dist.begin(), child_dist.end());

        // Visit children in order
        for (auto [dist, child_idx] : child_dist) {
            query_nearest_recursive(child_idx, point, k, candidates);
        }
    }
}

BoundingBox Octree::get_bounding_box() const {
    if (!is_built() || nodes_.empty()) {
        return BoundingBox();
    }
    return nodes_[0].bbox;
}

Octree::Statistics Octree::get_statistics() const {
    Statistics stats = {
        .total_nodes = 0,
        .leaf_nodes = 0,
        .internal_nodes = 0,
        .max_tree_depth = 0,
        .avg_simplices_per_leaf = 0,
        .max_simplices_per_leaf = 0,
        .empty_nodes = 0
    };

    if (!is_built() || nodes_.empty()) {
        return stats;
    }

    compute_statistics_recursive(0, stats, 0);

    if (stats.leaf_nodes > 0) {
        stats.avg_simplices_per_leaf = stats.avg_simplices_per_leaf / stats.leaf_nodes;
    }

    return stats;
}

void Octree::compute_statistics_recursive(size_t node_idx, Statistics& stats, int current_depth) const {
    const OctreeNode& node = nodes_[node_idx];

    stats.total_nodes++;
    stats.max_tree_depth = std::max(stats.max_tree_depth, static_cast<size_t>(current_depth));

    if (node.simplices.empty()) {
        stats.empty_nodes++;
    }

    if (node.is_leaf) {
        stats.leaf_nodes++;
        size_t count = node.simplices.size();
        stats.avg_simplices_per_leaf += count;
        stats.max_simplices_per_leaf = std::max(stats.max_simplices_per_leaf, count);
    } else {
        stats.internal_nodes++;
        for (int octant = 0; octant < 8; ++octant) {
            size_t child_idx = node.get_child(octant);
            if (child_idx != SIZE_MAX) {
                compute_statistics_recursive(child_idx, stats, current_depth + 1);
            }
        }
    }
}

} // namespace cebu
