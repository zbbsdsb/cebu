#include "cebu/bvh.h"
#include <algorithm>
#include <queue>
#include <cmath>
#include <limits>

namespace cebu {

// BVHTree implementation
BVHTree::BVHTree(
    BVHBuildStrategy strategy,
    size_t max_simplices_per_node,
    size_t max_depth
)
    : strategy_(strategy),
      max_simplices_per_node_(max_simplices_per_node),
      max_depth_(max_depth),
      vertex_geometry_(nullptr),
      get_simplex_vertices_(nullptr) {
}

void BVHTree::build(
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

    // Build the tree
    size_t root = build_recursive(simplices, 0);

    if (nodes_.empty() || root == SIZE_MAX) {
        clear();
    }
}

void BVHTree::rebuild() {
    if (simplex_ids_.empty() || !vertex_geometry_ || !get_simplex_vertices_) {
        return;
    }

    // Store the current simplices
    std::vector<SimplexID> simplices = simplex_ids_;

    // Rebuild
    build(simplices, *vertex_geometry_, get_simplex_vertices_);
}

void BVHTree::add_simplex(SimplexID simplex_id) {
    // Simple incremental update: add to simplex list and rebuild
    simplex_ids_.push_back(simplex_id);

    // For efficiency, only rebuild periodically or when needed
    // For now, we'll use a simple approach: rebuild if the tree is small enough
    if (simplex_ids_.size() < max_simplices_per_node_ * 4) {
        rebuild();
    }
}

void BVHTree::remove_simplex(SimplexID simplex_id) {
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

void BVHTree::clear() {
    nodes_.clear();
    simplex_ids_.clear();
    simplex_to_leaves_.clear();
}

size_t BVHTree::create_node() {
    size_t idx = nodes_.size();
    nodes_.emplace_back();
    return idx;
}

size_t BVHTree::build_recursive(const std::vector<SimplexID>& simplex_ids, int depth) {
    // Create a new node
    size_t node_idx = create_node();
    BVHNode& node = nodes_[node_idx];
    node.depth = depth;

    // Compute bounding box for all simplices
    node.bbox = compute_bbox(simplex_ids);

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

    // Choose build strategy and create internal node
    switch (strategy_) {
        case BVHBuildStrategy::MEDIAN_SPLIT:
            return build_median_split(simplex_ids, depth);
        case BVHBuildStrategy::SAH:
            return build_sah(simplex_ids, depth);
        case BVHBuildStrategy::MIDPOINT_SPLIT:
            return build_midpoint_split(simplex_ids, depth);
        case BVHBuildStrategy::EQUAL_COUNTS:
            return build_equal_counts(simplex_ids, depth);
        case BVHBuildStrategy::HLBVH:
            // Fall back to SAH for HLBVH (not implemented)
            return build_sah(simplex_ids, depth);
        default:
            return build_median_split(simplex_ids, depth);
    }
}

size_t BVHTree::build_median_split(const std::vector<SimplexID>& simplex_ids, int depth) {
    size_t node_idx = create_node();
    BVHNode& node = nodes_[node_idx];
    node.depth = depth;

    if (simplex_ids.size() <= max_simplices_per_node_) {
        node.is_leaf = true;
        node.simplices = simplex_ids;
        return node_idx;
    }

    // Compute bounding box
    node.bbox = compute_bbox(simplex_ids);

    // Find the axis with the largest extent
    int axis = get_longest_axis(node.bbox);

    // Sort simplices by centroid along the axis
    std::vector<SimplexID> sorted = simplex_ids;
    std::sort(sorted.begin(), sorted.end(), [this, axis](SimplexID a, SimplexID b) {
        Point3D ca = get_simplex_centroid(a);
        Point3D cb = get_simplex_centroid(b);
        switch (axis) {
            case 0: return ca.x < cb.x;
            case 1: return ca.y < cb.y;
            case 2: return ca.z < cb.z;
            default: return ca.x < cb.x;
        }
    });

    // Split at median
    size_t mid = sorted.size() / 2;
    std::vector<SimplexID> left(sorted.begin(), sorted.begin() + mid);
    std::vector<SimplexID> right(sorted.begin() + mid, sorted.end());

    // Recursively build children
    node.left_child = build_recursive(left, depth + 1);
    node.right_child = build_recursive(right, depth + 1);
    node.is_leaf = false;

    return node_idx;
}

size_t BVHTree::build_sah(const std::vector<SimplexID>& simplex_ids, int depth) {
    size_t node_idx = create_node();
    BVHNode& node = nodes_[node_idx];
    node.depth = depth;

    if (simplex_ids.size() <= max_simplices_per_node_) {
        node.is_leaf = true;
        node.simplices = simplex_ids;
        return node_idx;
    }

    // Compute bounding box
    node.bbox = compute_bbox(simplex_ids);

    // Try all axes and find the best split using SAH
    float best_cost = std::numeric_limits<float>::max();
    int best_axis = 0;
    float best_split = 0.0f;
    std::vector<SimplexID> best_left, best_right;

    for (int axis = 0; axis < 3; ++axis) {
        // Sort simplices by centroid along the axis
        std::vector<SimplexID> sorted = simplex_ids;
        std::sort(sorted.begin(), sorted.end(), [this, axis](SimplexID a, SimplexID b) {
            Point3D ca = get_simplex_centroid(a);
            Point3D cb = get_simplex_centroid(b);
            switch (axis) {
                case 0: return ca.x < cb.x;
                case 1: return ca.y < cb.y;
                case 2: return ca.z < cb.z;
                default: return ca.x < cb.x;
            }
        });

        // Try split at each position
        for (size_t i = 1; i < sorted.size(); ++i) {
            std::vector<SimplexID> left(sorted.begin(), sorted.begin() + i);
            std::vector<SimplexID> right(sorted.begin() + i, sorted.end());

            BoundingBox left_bbox = compute_bbox(left);
            BoundingBox right_bbox = compute_bbox(right);

            float cost = left_bbox.surface_area() * left.size() +
                         right_bbox.surface_area() * right.size();

            if (cost < best_cost) {
                best_cost = cost;
                best_axis = axis;
                best_left = left;
                best_right = right;
            }
        }
    }

    // If no good split found, create leaf
    if (best_left.empty() || best_right.empty()) {
        node.is_leaf = true;
        node.simplices = simplex_ids;
        return node_idx;
    }

    // Recursively build children
    node.left_child = build_recursive(best_left, depth + 1);
    node.right_child = build_recursive(best_right, depth + 1);
    node.is_leaf = false;

    return node_idx;
}

size_t BVHTree::build_midpoint_split(const std::vector<SimplexID>& simplex_ids, int depth) {
    size_t node_idx = create_node();
    BVHNode& node = nodes_[node_idx];
    node.depth = depth;

    if (simplex_ids.size() <= max_simplices_per_node_) {
        node.is_leaf = true;
        node.simplices = simplex_ids;
        return node_idx;
    }

    // Compute bounding box
    node.bbox = compute_bbox(simplex_ids);

    // Find the axis with the largest extent
    int axis = get_longest_axis(node.bbox);

    // Compute midpoint
    float midpoint;
    switch (axis) {
        case 0: midpoint = (node.bbox.min.x + node.bbox.max.x) * 0.5f; break;
        case 1: midpoint = (node.bbox.min.y + node.bbox.max.y) * 0.5f; break;
        case 2: midpoint = (node.bbox.min.z + node.bbox.max.z) * 0.5f; break;
        default: midpoint = 0.0f;
    }

    // Partition by midpoint
    std::vector<SimplexID> left, right;
    partition_by_axis(simplex_ids, axis, midpoint, left, right);

    // If partitioning failed, create leaf
    if (left.empty() || right.empty()) {
        node.is_leaf = true;
        node.simplices = simplex_ids;
        return node_idx;
    }

    // Recursively build children
    node.left_child = build_recursive(left, depth + 1);
    node.right_child = build_recursive(right, depth + 1);
    node.is_leaf = false;

    return node_idx;
}

size_t BVHTree::build_equal_counts(const std::vector<SimplexID>& simplex_ids, int depth) {
    size_t node_idx = create_node();
    BVHNode& node = nodes_[node_idx];
    node.depth = depth;

    if (simplex_ids.size() <= max_simplices_per_node_) {
        node.is_leaf = true;
        node.simplices = simplex_ids;
        return node_idx;
    }

    // Compute bounding box
    node.bbox = compute_bbox(simplex_ids);

    // Find the axis with the largest extent
    int axis = get_longest_axis(node.bbox);

    // Sort simplices by centroid along the axis
    std::vector<SimplexID> sorted = simplex_ids;
    std::sort(sorted.begin(), sorted.end(), [this, axis](SimplexID a, SimplexID b) {
        Point3D ca = get_simplex_centroid(a);
        Point3D cb = get_simplex_centroid(b);
        switch (axis) {
            case 0: return ca.x < cb.x;
            case 1: return ca.y < cb.y;
            case 2: return ca.z < cb.z;
            default: return ca.x < cb.x;
        }
    });

    // Split to equal halves
    size_t mid = sorted.size() / 2;
    std::vector<SimplexID> left(sorted.begin(), sorted.begin() + mid);
    std::vector<SimplexID> right(sorted.begin() + mid, sorted.end());

    // Recursively build children
    node.left_child = build_recursive(left, depth + 1);
    node.right_child = build_recursive(right, depth + 1);
    node.is_leaf = false;

    return node_idx;
}

BoundingBox BVHTree::compute_bbox(const std::vector<SimplexID>& simplex_ids) const {
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

int BVHTree::get_longest_axis(const BoundingBox& bbox) const {
    float dx = bbox.extent(0);
    float dy = bbox.extent(1);
    float dz = bbox.extent(2);

    if (dx >= dy && dx >= dz) return 0;
    if (dy >= dx && dy >= dz) return 1;
    return 2;
}

void BVHTree::partition_by_axis(
    const std::vector<SimplexID>& simplex_ids,
    int axis,
    float split_point,
    std::vector<SimplexID>& left,
    std::vector<SimplexID>& right
) const {
    left.clear();
    right.clear();
    left.reserve(simplex_ids.size() / 2);
    right.reserve(simplex_ids.size() / 2);

    for (SimplexID id : simplex_ids) {
        Point3D centroid = get_simplex_centroid(id);
        float value;
        switch (axis) {
            case 0: value = centroid.x; break;
            case 1: value = centroid.y; break;
            case 2: value = centroid.z; break;
            default: value = centroid.x;
        }

        if (value < split_point) {
            left.push_back(id);
        } else {
            right.push_back(id);
        }
    }
}

Point3D BVHTree::get_simplex_centroid(SimplexID simplex_id) const {
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

std::vector<SimplexID> BVHTree::query_range(const BoundingBox& bbox) const {
    std::vector<SimplexID> results;
    if (!is_built() || nodes_.empty()) {
        return results;
    }
    query_range_recursive(0, bbox, results);
    return results;
}

void BVHTree::query_range_recursive(size_t node_idx, const BoundingBox& bbox,
                                     std::vector<SimplexID>& results) const {
    const BVHNode& node = nodes_[node_idx];

    // Check if this node's bbox intersects with query bbox
    if (!node.bbox.intersects(bbox)) {
        return;
    }

    if (node.is_leaf) {
        // Check each simplex in the leaf
        for (SimplexID id : node.simplices) {
            // Check if simplex bbox intersects with query bbox
            // For now, we'll add all simplices in intersecting leaves
            results.push_back(id);
        }
    } else {
        // Recursively query children
        if (node.has_left_child()) {
            query_range_recursive(node.left_child, bbox, results);
        }
        if (node.has_right_child()) {
            query_range_recursive(node.right_child, bbox, results);
        }
    }
}

std::vector<SimplexID> BVHTree::query_nearest(const Point3D& point, size_t k) const {
    std::vector<SimplexID> results;
    if (!is_built() || nodes_.empty() || k == 0) {
        return results;
    }

    // Use priority queue for k-NN search
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

void BVHTree::query_nearest_recursive(size_t node_idx, const Point3D& point,
                                      size_t k, std::vector<std::pair<float, SimplexID>>& candidates) const {
    const BVHNode& node = nodes_[node_idx];

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
        // Visit closer child first
        float dist_left = std::numeric_limits<float>::max();
        float dist_right = std::numeric_limits<float>::max();

        if (node.has_left_child()) {
            const BVHNode& left = nodes_[node.left_child];
            Point3D left_center = left.bbox.center();
            dist_left = point.squared_distance_to(left_center);
        }
        if (node.has_right_child()) {
            const BVHNode& right = nodes_[node.right_child];
            Point3D right_center = right.bbox.center();
            dist_right = point.squared_distance_to(right_center);
        }

        if (dist_left < dist_right) {
            if (node.has_left_child()) {
                query_nearest_recursive(node.left_child, point, k, candidates);
            }
            if (node.has_right_child()) {
                query_nearest_recursive(node.right_child, point, k, candidates);
            }
        } else {
            if (node.has_right_child()) {
                query_nearest_recursive(node.right_child, point, k, candidates);
            }
            if (node.has_left_child()) {
                query_nearest_recursive(node.left_child, point, k, candidates);
            }
        }
    }
}

std::vector<SimplexID> BVHTree::query_ray(const Point3D& origin, const Point3D& direction) const {
    std::vector<SimplexID> results;
    if (!is_built() || nodes_.empty()) {
        return results;
    }
    query_ray_recursive(0, origin, direction, results);
    return results;
}

void BVHTree::query_ray_recursive(size_t node_idx, const Point3D& origin,
                                   const Point3D& direction, std::vector<SimplexID>& results) const {
    const BVHNode& node = nodes_[node_idx];

    // Check if ray intersects with node's bbox
    if (!ray_intersects_bbox(origin, direction, node.bbox)) {
        return;
    }

    if (node.is_leaf) {
        // Add all simplices in the leaf (actual intersection test would be more sophisticated)
        for (SimplexID id : node.simplices) {
            results.push_back(id);
        }
    } else {
        // Recursively query children
        if (node.has_left_child()) {
            query_ray_recursive(node.left_child, origin, direction, results);
        }
        if (node.has_right_child()) {
            query_ray_recursive(node.right_child, origin, direction, results);
        }
    }
}

bool BVHTree::ray_intersects_bbox(const Point3D& origin, const Point3D& direction,
                                   const BoundingBox& bbox) const {
    float t_min = 0.0f;
    float t_max = std::numeric_limits<float>::max();

    for (int i = 0; i < 3; ++i) {
        float min_val, max_val;
        float origin_val, dir_val;

        switch (i) {
            case 0:
                min_val = bbox.min.x;
                max_val = bbox.max.x;
                origin_val = origin.x;
                dir_val = direction.x;
                break;
            case 1:
                min_val = bbox.min.y;
                max_val = bbox.max.y;
                origin_val = origin.y;
                dir_val = direction.y;
                break;
            case 2:
                min_val = bbox.min.z;
                max_val = bbox.max.z;
                origin_val = origin.z;
                dir_val = direction.z;
                break;
            default:
                continue;
        }

        if (std::abs(dir_val) < 1e-6f) {
            // Ray is parallel to this axis
            if (origin_val < min_val || origin_val > max_val) {
                return false;
            }
        } else {
            float t1 = (min_val - origin_val) / dir_val;
            float t2 = (max_val - origin_val) / dir_val;

            if (t1 > t2) {
                std::swap(t1, t2);
            }

            t_min = std::max(t_min, t1);
            t_max = std::min(t_max, t2);

            if (t_min > t_max) {
                return false;
            }
        }
    }

    return true;
}

bool BVHTree::simplex_contains_point(const Point3D& point, SimplexID simplex_id) const {
    // For now, we'll use a simple centroid-based check
    // A proper implementation would use barycentric coordinates or point-in-simplex test
    Point3D centroid = get_simplex_centroid(simplex_id);
    float threshold = 0.5f; // Threshold for "near" the simplex
    return point.distance_to(centroid) < threshold;
}

BoundingBox BVHTree::get_bounding_box() const {
    if (!is_built() || nodes_.empty()) {
        return BoundingBox();
    }
    return nodes_[0].bbox;
}

BVHTree::Statistics BVHTree::get_statistics() const {
    Statistics stats = {
        .total_nodes = 0,
        .leaf_nodes = 0,
        .internal_nodes = 0,
        .max_tree_depth = 0,
        .avg_simplices_per_leaf = 0,
        .max_simplices_per_leaf = 0
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

void BVHTree::compute_statistics_recursive(size_t node_idx, Statistics& stats, int current_depth) const {
    const BVHNode& node = nodes_[node_idx];

    stats.total_nodes++;
    stats.max_tree_depth = std::max(stats.max_tree_depth, static_cast<size_t>(current_depth));

    if (node.is_leaf) {
        stats.leaf_nodes++;
        size_t count = node.simplices.size();
        stats.avg_simplices_per_leaf += count;
        stats.max_simplices_per_leaf = std::max(stats.max_simplices_per_leaf, count);
    } else {
        stats.internal_nodes++;
        if (node.has_left_child()) {
            compute_statistics_recursive(node.left_child, stats, current_depth + 1);
        }
        if (node.has_right_child()) {
            compute_statistics_recursive(node.right_child, stats, current_depth + 1);
        }
    }
}

} // namespace cebu
