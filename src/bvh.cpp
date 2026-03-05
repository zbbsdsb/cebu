#include "cebu/bvh.h"
#include <algorithm>
#include <numeric>
#include <queue>
#include <chrono>
#include <iostream>

namespace cebu {

BVH::BVH(const SimplicialComplex& complex,
          BuildStrategy strategy,
          size_t max_simplices_per_leaf,
          size_t max_depth)
    : complex_(&complex),
      strategy_(strategy),
      max_simplices_per_leaf_(max_simplices_per_leaf),
      max_depth_(max_depth) {

    // Collect all simplices
    std::vector<SimplexID> all_simplices;
    for (const auto& [sid, simplex] : complex_->get_all_simplices()) {
        all_simplices.push_back(sid);
        // Pre-compute simplex centers
        auto bbox = compute_bounding_box(sid);
        auto center = compute_center(bbox);
        simplex_centers_[sid] = {center[0], center[1], center[2]};
    }

    // Build tree
    root_ = build_tree(all_simplices, 0);
}

void BVH::rebuild() {
    // Clear existing tree
    root_.reset();
    simplex_centers_.clear();

    // Collect all simplices
    std::vector<SimplexID> all_simplices;
    for (const auto& [sid, simplex] : complex_->get_all_simplices()) {
        all_simplices.push_back(sid);
        auto bbox = compute_bounding_box(sid);
        auto center = compute_center(bbox);
        simplex_centers_[sid] = {center[0], center[1], center[2]};
    }

    // Rebuild tree
    root_ = build_tree(all_simplices, 0);
}

std::optional<std::array<float, 3>> BVH::get_vertex_position(VertexID vid) const {
    // Note: This is a placeholder - actual implementation depends on
    // how vertex positions are stored in your complex
    // For now, return empty
    return std::nullopt;
}

BVH::BoundingBox BVH::compute_bounding_box(SimplexID sid) const {
    BoundingBox bbox = {std::numeric_limits<float>::max(),
                       std::numeric_limits<float>::max(),
                       std::numeric_limits<float>::max(),
                       std::numeric_limits<float>::lowest(),
                       std::numeric_limits<float>::lowest(),
                       std::numeric_limits<float>::lowest()};

    // Note: This is a simplified implementation
    // Actual implementation should use vertex positions from the complex
    // For now, use simplex centers as a fallback

    auto it = simplex_centers_.find(sid);
    if (it != simplex_centers_.end()) {
        float x = it->second[0];
        float y = it->second[1];
        float z = it->second[2];

        bbox[0] = std::min(bbox[0], x - 0.5f);
        bbox[1] = std::min(bbox[1], y - 0.5f);
        bbox[2] = std::min(bbox[2], z - 0.5f);
        bbox[3] = std::max(bbox[3], x + 0.5f);
        bbox[4] = std::max(bbox[4], y + 0.5f);
        bbox[5] = std::max(bbox[5], z + 0.5f);
    }

    return bbox;
}

BVH::BoundingBox BVH::compute_bounding_box(const std::vector<SimplexID>& simplices) const {
    if (simplices.empty()) {
        return BoundingBox{};
    }

    BoundingBox combined = compute_bounding_box(simplices[0]);
    for (size_t i = 1; i < simplices.size(); ++i) {
        combined = merge_bounding_boxes(combined, compute_bounding_box(simplices[i]));
    }
    return combined;
}

BVH::BoundingBox BVH::merge_bounding_boxes(const BoundingBox& a,
                                            const BoundingBox& b) const {
    BoundingBox merged = a;
    merged[0] = std::min(a[0], b[0]);
    merged[1] = std::min(a[1], b[1]);
    merged[2] = std::min(a[2], b[2]);
    merged[3] = std::max(a[3], b[3]);
    merged[4] = std::max(a[4], b[4]);
    merged[5] = std::max(a[5], b[5]);
    return merged;
}

float BVH::compute_surface_area(const BoundingBox& bbox) const {
    float dx = bbox[3] - bbox[0];
    float dy = bbox[4] - bbox[1];
    float dz = bbox[5] - bbox[2];

    if (dx < 0 || dy < 0 || dz < 0) {
        return 0.0f;
    }

    return 2.0f * (dx * dy + dy * dz + dz * dx);
}

bool BVH::intersects_point(const BoundingBox& bbox, float x, float y, float z) const {
    return x >= bbox[0] && x <= bbox[3] &&
           y >= bbox[1] && y <= bbox[4] &&
           z >= bbox[2] && z <= bbox[5];
}

bool BVH::intersects_range(const BoundingBox& bbox,
                           float min_x, float min_y, float min_z,
                           float max_x, float max_y, float max_z) const {
    return !(bbox[3] < min_x || bbox[0] > max_x ||
             bbox[4] < min_y || bbox[1] > max_y ||
             bbox[5] < min_z || bbox[2] > max_z);
}

bool BVH::intersects_sphere(const BoundingBox& bbox,
                            float center_x, float center_y, float center_z,
                            float radius) const {
    // Find closest point on bbox to sphere center
    float closest_x = std::max(bbox[0], std::min(center_x, bbox[3]));
    float closest_y = std::max(bbox[1], std::min(center_y, bbox[4]));
    float closest_z = std::max(bbox[2], std::min(center_z, bbox[5]));

    // Compute distance
    float dx = center_x - closest_x;
    float dy = center_y - closest_y;
    float dz = center_z - closest_z;

    float distance_sq = dx * dx + dy * dy + dz * dz;
    return distance_sq <= radius * radius;
}

std::array<float, 3> BVH::compute_center(const BoundingBox& bbox) const {
    return {(bbox[0] + bbox[3]) * 0.5f,
            (bbox[1] + bbox[4]) * 0.5f,
            (bbox[2] + bbox[5]) * 0.5f};
}

size_t BVH::compute_longest_axis(const BoundingBox& bbox) const {
    float dx = bbox[3] - bbox[0];
    float dy = bbox[4] - bbox[1];
    float dz = bbox[5] - bbox[2];

    if (dx >= dy && dx >= dz) return 0;  // X-axis
    if (dy >= dz) return 1;              // Y-axis
    return 2;                           // Z-axis
}

std::pair<std::vector<SimplexID>, std::vector<SimplexID>>
BVH::split_by_median(const std::vector<SimplexID>& simplices, size_t axis) {
    if (simplices.empty()) {
        return {};
    }

    // Copy and sort by center along axis
    std::vector<SimplexID> sorted = simplices;
    std::sort(sorted.begin(), sorted.end(),
              [this, axis](SimplexID a, SimplexID b) {
                  auto it_a = simplex_centers_.find(a);
                  auto it_b = simplex_centers_.find(b);
                  if (it_a == simplex_centers_.end() || it_b == simplex_centers_.end()) {
                      return false;
                  }
                  return it_a->second[axis] < it_b->second[axis];
              });

    // Split at median
    size_t mid = sorted.size() / 2;
    std::vector<SimplexID> left(sorted.begin(), sorted.begin() + mid);
    std::vector<SimplexID> right(sorted.begin() + mid, sorted.end());

    return {left, right};
}

std::pair<std::vector<SimplexID>, std::vector<SimplexID>>
BVH::split_by_midpoint(const std::vector<SimplexID>& simplices, size_t axis, float midpoint) {
    std::vector<SimplexID> left, right;

    for (SimplexID sid : simplices) {
        auto it = simplex_centers_.find(sid);
        if (it != simplex_centers_.end()) {
            if (it->second[axis] <= midpoint) {
                left.push_back(sid);
            } else {
                right.push_back(sid);
            }
        } else {
            right.push_back(sid);
        }
    }

    return {left, right};
}

std::pair<std::vector<SimplexID>, std::vector<SimplexID>>
BVH::split_by_sah(const std::vector<SimplexID>& simplices) {
    // Simplified SAH: try all three axes and pick the best split
    BoundingBox total_bbox = compute_bounding_box(simplices);
    float total_area = compute_surface_area(total_bbox);
    float total_cost = simplices.size() * total_area;

    float best_cost = std::numeric_limits<float>::max();
    size_t best_axis = 0;
    std::pair<std::vector<SimplexID>, std::vector<SimplexID>> best_split;

    for (size_t axis = 0; axis < 3; ++axis) {
        auto [left, right] = split_by_median(simplices, axis);

        if (left.empty() || right.empty()) continue;

        BoundingBox left_bbox = compute_bounding_box(left);
        BoundingBox right_bbox = compute_bounding_box(right);

        float left_area = compute_surface_area(left_bbox);
        float right_area = compute_surface_area(right_bbox);

        float sah_cost = left.size() * left_area + right.size() * right_area;

        if (sah_cost < best_cost) {
            best_cost = sah_cost;
            best_axis = axis;
            best_split = {left, right};
        }
    }

    return best_split;
}

std::unique_ptr<BVH::BVHNode> BVH::build_tree(const std::vector<SimplexID>& simplices,
                                               size_t depth) {
    auto node = std::make_unique<BVHNode>();
    node->depth = depth;

    // Compute bounding box
    node->bbox = compute_bounding_box(simplices);

    // Check termination conditions
    if (simplices.size() <= max_simplices_per_leaf_ || depth >= max_depth_) {
        node->is_leaf = true;
        node->simplices = simplices;
        return node;
    }

    // Split simplices
    std::pair<std::vector<SimplexID>, std::vector<SimplexID>> split;

    switch (strategy_) {
        case BuildStrategy::MEDIAN_SPLIT: {
            size_t axis = compute_longest_axis(node->bbox);
            split = split_by_median(simplices, axis);
            break;
        }
        case BuildStrategy::SAH: {
            split = split_by_sah(simplices);
            break;
        }
        case BuildStrategy::MIDPOINT_SPLIT: {
            size_t axis = compute_longest_axis(node->bbox);
            auto center = compute_center(node->bbox);
            split = split_by_midpoint(simplices, axis, center[axis]);
            break;
        }
    }

    // Handle empty split
    if (split.first.empty() || split.second.empty()) {
        node->is_leaf = true;
        node->simplices = simplices;
        return node;
    }

    // Build children
    node->left = build_tree(split.first, depth + 1);
    node->right = build_tree(split.second, depth + 1);
    node->is_leaf = false;

    return node;
}

BVH::QueryResult BVH::range_query(float min_x, float min_y, float min_z,
                                   float max_x, float max_y, float max_z) {
    QueryResult result;

    if (!root_) {
        return result;
    }

    auto start = std::chrono::high_resolution_clock::now();
    range_query_helper(root_.get(), min_x, min_y, min_z, max_x, max_y, max_z,
                       result.found, result.nodes_visited);
    auto end = std::chrono::high_resolution_clock::now();

    result.query_time_ms = std::chrono::duration<double, std::milli>(end - start).count();
    return result;
}

void BVH::range_query_helper(const BVHNode* node,
                              float min_x, float min_y, float min_z,
                              float max_x, float max_y, float max_z,
                              std::vector<SimplexID>& results,
                              size_t& nodes_visited) const {
    if (!node) {
        return;
    }

    nodes_visited++;

    // Check if node intersects query range
    if (!intersects_range(node->bbox, min_x, min_y, min_z, max_x, max_y, max_z)) {
        return;
    }

    // If leaf node, check all simplices
    if (node->is_leaf) {
        for (SimplexID sid : node->simplices) {
            // Check if simplex center is in range
            auto it = simplex_centers_.find(sid);
            if (it != simplex_centers_.end()) {
                float x = it->second[0];
                float y = it->second[1];
                float z = it->second[2];

                if (x >= min_x && x <= max_x &&
                    y >= min_y && y <= max_y &&
                    z >= min_z && z <= max_z) {
                    results.push_back(sid);
                }
            }
        }
        return;
    }

    // Recursively query children
    range_query_helper(node->left.get(), min_x, min_y, min_z, max_x, max_y, max_z,
                       results, nodes_visited);
    range_query_helper(node->right.get(), min_x, min_y, min_z, max_x, max_y, max_z,
                       results, nodes_visited);
}

BVH::NearestResult BVH::nearest_neighbor(float x, float y, float z) {
    NearestResult result;

    if (!root_) {
        return result;
    }

    nearest_neighbor_helper(root_.get(), x, y, z, result);
    return result;
}

void BVH::nearest_neighbor_helper(const BVHNode* node,
                                   float x, float y, float z,
                                   NearestResult& result) const {
    if (!node) {
        return;
    }

    result.nodes_visited++;

    // Prune if current best is better than this node's bbox
    float dist_to_bbox = distance_to_bbox(node->bbox, x, y, z);
    if (dist_to_bbox >= result.distance) {
        return;
    }

    // If leaf node, check all simplices
    if (node->is_leaf) {
        for (SimplexID sid : node->simplices) {
            float dist = distance_to_simplex(sid, x, y, z);
            if (dist < result.distance) {
                result.distance = dist;
                result.simplex_id = sid;
            }
        }
        return;
    }

    // Query children in order of distance
    std::array<std::pair<float, BVHNode*>, 2> children = {
        {{distance_to_bbox(node->left->bbox, x, y, z), node->left.get()},
         {distance_to_bbox(node->right->bbox, x, y, z), node->right.get()}}
    };

    std::sort(children.begin(), children.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });

    for (const auto& [dist, child] : children) {
        if (dist < result.distance) {
            nearest_neighbor_helper(child, x, y, z, result);
        }
    }
}

float BVH::distance_to_bbox(const BoundingBox& bbox,
                             float x, float y, float z) const {
    // Find closest point on bbox
    float closest_x = std::max(bbox[0], std::min(x, bbox[3]));
    float closest_y = std::max(bbox[1], std::min(y, bbox[4]));
    float closest_z = std::max(bbox[2], std::min(z, bbox[5]));

    float dx = x - closest_x;
    float dy = y - closest_y;
    float dz = z - closest_z;

    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

float BVH::distance_to_simplex(SimplexID sid, float x, float y, float z) const {
    // Simplified: use distance to simplex center
    auto it = simplex_centers_.find(sid);
    if (it != simplex_centers_.end()) {
        float dx = x - it->second[0];
        float dy = y - it->second[1];
        float dz = z - it->second[2];
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }
    return std::numeric_limits<float>::max();
}

void BVH::incremental_update(const std::vector<SimplexID>& added,
                             const std::vector<SimplexID>& removed,
                             const std::vector<SimplexID>& modified) {
    // For simplicity, just rebuild the entire tree
    // A more sophisticated implementation would do incremental updates
    rebuild();
}

std::vector<std::pair<SimplexID, double>>
BVH::k_nearest_neighbors(float x, float y, float z, size_t k) {
    // Simplified implementation: use nearest_neighbor k times
    std::vector<std::pair<SimplexID, double>> results;

    // For a proper implementation, we would use a priority queue
    // and traverse the tree once
    NearestResult result = nearest_neighbor(x, y, z);
    if (result.simplex_id) {
        results.emplace_back(*result.simplex_id, result.distance);
    }

    return results;
}

BVH::QueryResult BVH::point_query(float x, float y, float z) {
    QueryResult result;

    if (!root_) {
        return result;
    }

    auto start = std::chrono::high_resolution_clock::now();
    point_query_helper(root_.get(), x, y, z, result.found, result.nodes_visited);
    auto end = std::chrono::high_resolution_clock::now();

    result.query_time_ms = std::chrono::duration<double, std::milli>(end - start).count();
    return result;
}

void BVH::point_query_helper(const BVHNode* node,
                             float x, float y, float z,
                             std::vector<SimplexID>& results,
                             size_t& nodes_visited) const {
    if (!node) {
        return;
    }

    nodes_visited++;

    // Check if point is in node's bbox
    if (!intersects_point(node->bbox, x, y, z)) {
        return;
    }

    // If leaf node, check all simplices
    if (node->is_leaf) {
        for (SimplexID sid : node->simplices) {
            auto it = simplex_centers_.find(sid);
            if (it != simplex_centers_.end()) {
                // Check if point is close to simplex center
                float dx = x - it->second[0];
                float dy = y - it->second[1];
                float dz = z - it->second[2];
                float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
                if (dist < 0.5f) {  // Threshold
                    results.push_back(sid);
                }
            }
        }
        return;
    }

    // Recursively query children
    point_query_helper(node->left.get(), x, y, z, results, nodes_visited);
    point_query_helper(node->right.get(), x, y, z, results, nodes_visited);
}

BVH::QueryResult BVH::sphere_query(float center_x, float center_y, float center_z,
                                     float radius) {
    QueryResult result;

    if (!root_) {
        return result;
    }

    // Simplified: use range query with bounding box
    auto start = std::chrono::high_resolution_clock::now();

    float min_x = center_x - radius;
    float min_y = center_y - radius;
    float min_z = center_z - radius;
    float max_x = center_x + radius;
    float max_y = center_y + radius;
    float max_z = center_z + radius;

    std::vector<SimplexID> candidates;
    range_query_helper(root_.get(), min_x, min_y, min_z, max_x, max_y, max_z,
                       candidates, result.nodes_visited);

    // Filter by actual sphere intersection
    for (SimplexID sid : candidates) {
        auto it = simplex_centers_.find(sid);
        if (it != simplex_centers_.end()) {
            float dx = center_x - it->second[0];
            float dy = center_y - it->second[1];
            float dz = center_z - it->second[2];
            float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
            if (dist <= radius) {
                result.found.push_back(sid);
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    result.query_time_ms = std::chrono::duration<double, std::milli>(end - start).count();

    return result;
}

BVH::QueryResult BVH::ray_query(const std::array<float, 3>& origin,
                                const std::array<float, 3>& direction,
                                float t_max) {
    QueryResult result;

    if (!root_) {
        return result;
    }

    auto start = std::chrono::high_resolution_clock::now();
    ray_query_helper(root_.get(), origin, direction, t_max,
                     result.found, result.nodes_visited);
    auto end = std::chrono::high_resolution_clock::now();

    result.query_time_ms = std::chrono::duration<double, std::milli>(end - start).count();
    return result;
}

void BVH::ray_query_helper(const BVHNode* node,
                            const std::array<float, 3>& origin,
                            const std::array<float, 3>& direction,
                            float t_max,
                            std::vector<SimplexID>& results,
                            size_t& nodes_visited) const {
    if (!node) {
        return;
    }

    nodes_visited++;

    // Check if ray intersects node's bbox
    if (!ray_intersects_bbox(node->bbox, origin, direction, t_max)) {
        return;
    }

    // If leaf node, add all simplices (simplified)
    if (node->is_leaf) {
        for (SimplexID sid : node->simplices) {
            results.push_back(sid);
        }
        return;
    }

    // Recursively query children
    ray_query_helper(node->left.get(), origin, direction, t_max,
                      results, nodes_visited);
    ray_query_helper(node->right.get(), origin, direction, t_max,
                      results, nodes_visited);
}

bool BVH::ray_intersects_bbox(const BoundingBox& bbox,
                               const std::array<float, 3>& origin,
                               const std::array<float, 3>& direction,
                               float t_max) const {
    // Slab method for ray-box intersection
    float t_min = 0.0f;
    float t_inter_min = std::numeric_limits<float>::lowest();
    float t_inter_max = std::numeric_limits<float>::max();

    for (size_t i = 0; i < 3; ++i) {
        if (std::abs(direction[i]) < 1e-6f) {
            if (origin[i] < bbox[i] || origin[i] > bbox[i + 3]) {
                return false;
            }
        } else {
            float inv_d = 1.0f / direction[i];
            float t0 = (bbox[i] - origin[i]) * inv_d;
            float t1 = (bbox[i + 3] - origin[i]) * inv_d;

            if (inv_d < 0.0f) {
                std::swap(t0, t1);
            }

            t_inter_min = std::max(t_inter_min, t0);
            t_inter_max = std::min(t_inter_max, t1);
        }
    }

    return t_inter_max >= 0 && t_inter_min <= t_inter_max && t_inter_min <= t_max;
}

std::vector<std::pair<SimplexID, SimplexID>>
BVH::find_collisions(const BVH& other) const {
    std::vector<std::pair<SimplexID, SimplexID>> collisions;

    // Simplified implementation: collect all simplices from both trees
    // A proper implementation would traverse both trees simultaneously

    if (!root_ || !other.root_) {
        return collisions;
    }

    std::vector<SimplexID> simplices_a, simplices_b;

    std::function<void(const BVHNode*)> collect_simplices =
        [&simplices_a, &collect_simplices](const BVHNode* node) {
            if (!node) return;
            if (node->is_leaf) {
                simplices_a.insert(simplices_a.end(),
                                   node->simplices.begin(), node->simplices.end());
            } else {
                collect_simplices(node->left.get());
                collect_simplices(node->right.get());
            }
        };

    collect_simplices(root_.get());

    std::function<void(const BVHNode*)> collect_simplices_b =
        [&simplices_b, &collect_simplices_b](const BVHNode* node) {
            if (!node) return;
            if (node->is_leaf) {
                simplices_b.insert(simplices_b.end(),
                                   node->simplices.begin(), node->simplices.end());
            } else {
                collect_simplices_b(node->left.get());
                collect_simplices_b(node->right.get());
            }
        };

    collect_simplices_b(other.root_.get());

    // For now, return all pairs (simplified)
    for (SimplexID a : simplices_a) {
        for (SimplexID b : simplices_b) {
            collisions.emplace_back(a, b);
        }
    }

    return collisions;
}

std::unordered_map<std::string, size_t> BVH::get_statistics() const {
    std::unordered_map<std::string, size_t> stats;

    if (!root_) {
        return stats;
    }

    stats["max_depth"] = get_max_depth();
    stats["node_count"] = get_node_count();
    stats["leaf_count"] = get_leaf_count();

    return stats;
}

size_t BVH::get_max_depth() const {
    if (!root_) return 0;

    std::function<size_t(const BVHNode*)> max_depth_helper =
        [&max_depth_helper](const BVHNode* node) -> size_t {
        if (!node) return 0;
        if (node->is_leaf) return node->depth;
        return std::max(max_depth_helper(node->left.get()),
                        max_depth_helper(node->right.get()));
    };

    return max_depth_helper(root_.get());
}

size_t BVH::get_node_count() const {
    if (!root_) return 0;

    std::function<size_t(const BVHNode*)> count_nodes =
        [&count_nodes](const BVHNode* node) -> size_t {
        if (!node) return 0;
        return 1 + count_nodes(node->left.get()) + count_nodes(node->right.get());
    };

    return count_nodes(root_.get());
}

size_t BVH::get_leaf_count() const {
    if (!root_) return 0;

    std::function<size_t(const BVHNode*)> count_leaves =
        [&count_leaves](const BVHNode* node) -> size_t {
        if (!node) return 0;
        if (node->is_leaf) return 1;
        return count_leaves(node->left.get()) + count_leaves(node->right.get());
    };

    return count_leaves(root_.get());
}

bool BVH::is_valid() const {
    if (!root_) return true;

    std::function<bool(const BVHNode*)> validate =
        [this, &validate](const BVHNode* node) -> bool {
        if (!node) return true;

        // Check depth
        if (node->depth > max_depth_) {
            return false;
        }

        // Check bbox
        for (size_t i = 0; i < 3; ++i) {
            if (node->bbox[i] > node->bbox[i + 3]) {
                return false;
            }
        }

        if (node->is_leaf) {
            return !node->left && !node->right;
        } else {
            return node->left && node->right &&
                   validate(node->left.get()) && validate(node->right.get());
        }
    };

    return validate(root_.get());
}

void BVH::optimize() {
    // Simplified: just rebuild
    rebuild();
}

void BVH::clear() {
    root_.reset();
    simplex_centers_.clear();
}

} // namespace cebu
