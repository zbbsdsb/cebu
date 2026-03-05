#include "cebu/bvh.h"
#include <algorithm>
#include <queue>
#include <cmath>
#include <iostream>
#include <limits>

namespace cebu {

// ============================================================================
// BVH Implementation
// ============================================================================

BVH::BVH()
    : complex_(nullptr)
    , labeled_complex_(nullptr)
    , build_strategy_(BVHBuildStrategy::SAH)
    , max_leaf_size_(16)
    , max_depth_(32)
    , needs_rebuild_(false) {}

BVH::BVH(const SimplicialComplex& complex)
    : complex_(&complex)
    , labeled_complex_(nullptr)
    , build_strategy_(BVHBuildStrategy::SAH)
    , max_leaf_size_(16)
    , max_depth_(32)
    , needs_rebuild_(false) {
    build(complex);
}

BVH::BVH(const SimplicialComplexLabeled<>& complex)
    : complex_(nullptr)
    , labeled_complex_(&complex)
    , build_strategy_(BVHBuildStrategy::SAH)
    , max_leaf_size_(16)
    , max_depth_(32)
    , needs_rebuild_(false) {
    build(complex);
}

BVH::~BVH() = default;

void BVH::build(const SimplicialComplex& complex) {
    complex_ = &complex;
    labeled_complex_ = nullptr;
    needs_rebuild_ = false;

    // Collect all simplex IDs
    std::vector<SimplexID> simplex_ids;
    simplex_ids.reserve(complex->simplex_count());

    for (const auto& [id, simplex] : complex->get_all_simplices()) {
        simplex_ids.push_back(id);
    }

    // Build BVH recursively
    root_ = std::make_unique<BVHNode>();
    build_recursive(root_.get(), simplex_ids, 0);
}

void BVH::build(const SimplicialComplexLabeled<>& complex) {
    labeled_complex_ = &complex;
    complex_ = nullptr;
    needs_rebuild_ = false;

    // Collect all simplex IDs
    std::vector<SimplexID> simplex_ids;
    simplex_ids.reserve(complex.simplex_count());

    for (const auto& [id, simplex] : complex.get_all_simplices()) {
        simplex_ids.push_back(id);
    }

    // Build BVH recursively
    root_ = std::make_unique<BVHNode>();
    build_recursive(root_.get(), simplex_ids, 0);
}

void BVH::rebuild() {
    rebuild(build_strategy_);
}

void BVH::rebuild(BVHBuildStrategy strategy) {
    build_strategy_ = strategy;
    needs_rebuild_ = false;

    if (complex_) {
        build(*complex_);
    } else if (labeled_complex_) {
        build(*labeled_complex_);
    }
}

void BVH::build_recursive(BVHNode* node, std::vector<SimplexID>& simplex_ids,
                         size_t depth) {
    if (simplex_ids.empty()) {
        return;
    }

    // Compute bounding box for current node
    BoundingBox merged_bbox;
    for (SimplexID id : simplex_ids) {
        BoundingBox bbox = compute_bounding_box(id);
        if (merged_bbox.simplex_id == 0) {
            merged_bbox = bbox;
        } else {
            merged_bbox.merge(bbox);
        }
    }
    node->bbox = merged_bbox;

    // Check termination conditions
    if (simplex_ids.size() <= max_leaf_size_ || depth >= max_depth_) {
        node->type = BVHNodeType::LEAF;
        node->simplices = std::move(simplex_ids);
        return;
    }

    node->type = BVHNodeType::INTERNAL;
    node->left = std::make_unique<BVHNode>();
    node->right = std::make_unique<BVHNode>();

    // Choose build strategy
    switch (build_strategy_) {
        case BVHBuildStrategy::MIDPOINT:
            build_midpoint(node, simplex_ids, depth);
            break;
        case BVHBuildStrategy::MEDIAN:
            build_median(node, simplex_ids, depth);
            break;
        case BVHBuildStrategy::SAH:
            build_sah(node, simplex_ids, depth);
            break;
        case BVHBuildStrategy::HILBERT:
            build_hilbert(node, simplex_ids, depth);
            break;
    }
}

void BVH::build_midpoint(BVHNode* node, std::vector<SimplexID>& simplex_ids, size_t depth) {
    // Find split axis (longest axis)
    std::array<double, 3> extents = {
        node->bbox.max[0] - node->bbox.min[0],
        node->bbox.max[1] - node->bbox.min[1],
        node->bbox.max[2] - node->bbox.min[2]
    };

    int axis = (extents[0] >= extents[1] && extents[0] >= extents[2]) ? 0 :
               (extents[1] >= extents[2]) ? 1 : 2;

    // Split at midpoint
    double midpoint = node->bbox.min[axis] + extents[axis] / 2.0;

    std::vector<SimplexID> left_ids, right_ids;
    for (SimplexID id : simplex_ids) {
        std::array<double, 3> pos = get_simplex_position(id);
        if (pos[axis] <= midpoint) {
            left_ids.push_back(id);
        } else {
            right_ids.push_back(id);
        }
    }

    // Handle degenerate case (all simplices on one side)
    if (left_ids.empty() || right_ids.empty()) {
        size_t mid = simplex_ids.size() / 2;
        left_ids.assign(simplex_ids.begin(), simplex_ids.begin() + mid);
        right_ids.assign(simplex_ids.begin() + mid, simplex_ids.end());
    }

    build_recursive(node->left.get(), left_ids, depth + 1);
    build_recursive(node->right.get(), right_ids, depth + 1);
}

void BVH::build_median(BVHNode* node, std::vector<SimplexID>& simplex_ids, size_t depth) {
    // Find split axis (longest axis)
    std::array<double, 3> extents = {
        node->bbox.max[0] - node->bbox.min[0],
        node->bbox.max[1] - node->bbox.min[1],
        node->bbox.max[2] - node->bbox.min[2]
    };

    int axis = (extents[0] >= extents[1] && extents[0] >= extents[2]) ? 0 :
               (extents[1] >= extents[2]) ? 1 : 2;

    // Sort by axis and split at median
    std::sort(simplex_ids.begin(), simplex_ids.end(),
              [this, axis](SimplexID a, SimplexID b) {
                  return get_simplex_position(a)[axis] < get_simplex_position(b)[axis];
              });

    size_t mid = simplex_ids.size() / 2;
    std::vector<SimplexID> left_ids(simplex_ids.begin(), simplex_ids.begin() + mid);
    std::vector<SimplexID> right_ids(simplex_ids.begin() + mid, simplex_ids.end());

    build_recursive(node->left.get(), left_ids, depth + 1);
    build_recursive(node->right.get(), right_ids, depth + 1);
}

void BVH::build_sah(BVHNode* node, std::vector<SimplexID>& simplex_ids, size_t depth) {
    if (simplex_ids.size() <= 4) {
        build_median(node, simplex_ids, depth);
        return;
    }

    // Try all axes and split positions
    double best_cost = std::numeric_limits<double>::max();
    int best_axis = 0;
    size_t best_split = simplex_ids.size() / 2;

    for (int axis = 0; axis < 3; ++axis) {
        // Sort by axis
        std::vector<SimplexID> sorted = simplex_ids;
        std::sort(sorted.begin(), sorted.end(),
                  [this, axis](SimplexID a, SimplexID b) {
                      return get_simplex_position(a)[axis] < get_simplex_position(b)[axis];
                  });

        // Try different split positions
        for (size_t split = 1; split < sorted.size(); ++split) {
            // Compute bounding boxes for left and right
            BoundingBox left_bbox, right_bbox;
            for (size_t i = 0; i < split; ++i) {
                BoundingBox bbox = compute_bounding_box(sorted[i]);
                if (left_bbox.simplex_id == 0) {
                    left_bbox = bbox;
                } else {
                    left_bbox.merge(bbox);
                }
            }
            for (size_t i = split; i < sorted.size(); ++i) {
                BoundingBox bbox = compute_bounding_box(sorted[i]);
                if (right_bbox.simplex_id == 0) {
                    right_bbox = bbox;
                } else {
                    right_bbox.merge(bbox);
                }
            }

            // Compute SAH cost
            double sa_left = left_bbox.surface_area();
            double sa_right = right_bbox.surface_area();
            double sa_node = node->bbox.surface_area();
            double cost = (split / sa_left + (sorted.size() - split) / sa_right) * sa_node;

            if (cost < best_cost) {
                best_cost = cost;
                best_axis = axis;
                best_split = split;
            }
        }
    }

    // Sort by best axis and split
    std::sort(simplex_ids.begin(), simplex_ids.end(),
              [this, best_axis](SimplexID a, SimplexID b) {
                  return get_simplex_position(a)[best_axis] < get_simplex_position(b)[best_axis];
              });

    std::vector<SimplexID> left_ids(simplex_ids.begin(), simplex_ids.begin() + best_split);
    std::vector<SimplexID> right_ids(simplex_ids.begin() + best_split, simplex_ids.end());

    build_recursive(node->left.get(), left_ids, depth + 1);
    build_recursive(node->right.get(), right_ids, depth + 1);
}

void BVH::build_hilbert(BVHNode* node, std::vector<SimplexID>& simplex_ids, size_t depth) {
    // Simplified Hilbert curve ordering (placeholder for full implementation)
    // For now, use median as a reasonable approximation
    build_median(node, simplex_ids, depth);
}

bool BVH::contains(const std::array<double, 3>& point) const {
    return !query_point(point).empty();
}

std::vector<SimplexID> BVH::query_point(const std::array<double, 3>& point) const {
    std::vector<SimplexID> results;
    if (root_) {
        query_point_recursive(root_.get(), point, results);
    }
    return results;
}

void BVH::query_point_recursive(const BVHNode* node, const std::array<double, 3>& point,
                               std::vector<SimplexID>& results) const {
    if (!node) {
        return;
    }

    if (!node->bbox.contains(point)) {
        return;
    }

    if (node->is_leaf()) {
        results.insert(results.end(), node->simplices.begin(), node->simplices.end());
    } else {
        query_point_recursive(node->left.get(), point, results);
        query_point_recursive(node->right.get(), point, results);
    }
}

BVHQueryResult BVH::query_range(const BoundingBox& range) const {
    BVHQueryResult result;
    if (root_) {
        query_range_recursive(root_.get(), range, result);
    }
    return result;
}

void BVH::query_range_recursive(const BVHNode* node, const BoundingBox& range,
                               BVHQueryResult& results) const {
    if (!node) {
        return;
    }

    results.nodes_visited++;

    if (!node->bbox.intersects(range)) {
        return;
    }

    if (node->is_leaf()) {
        for (SimplexID id : node->simplices) {
            BoundingBox bbox = compute_bounding_box(id);
            if (bbox.intersects(range)) {
                results.simplex_ids.push_back(id);
            }
        }
    } else {
        query_range_recursive(node->left.get(), range, results);
        query_range_recursive(node->right.get(), range, results);
    }
}

BVHQueryResult BVH::query_sphere(const std::array<double, 3>& center, double radius) const {
    BVHQueryResult result;
    double radius_sq = radius * radius;
    if (root_) {
        query_sphere_recursive(root_.get(), center, radius_sq, result);
    }
    return result;
}

void BVH::query_sphere_recursive(const BVHNode* node, const std::array<double, 3>& center,
                                double radius_sq, BVHQueryResult& results) const {
    if (!node) {
        return;
    }

    results.nodes_visited++;

    // Quick rejection: check if bounding box intersects sphere
    bool intersects = true;
    for (int i = 0; i < 3; ++i) {
        if (center[i] + sqrt(radius_sq) < node->bbox.min[i] ||
            center[i] - sqrt(radius_sq) > node->bbox.max[i]) {
            intersects = false;
            break;
        }
    }

    if (!intersects) {
        return;
    }

    if (node->is_leaf()) {
        for (SimplexID id : node->simplices) {
            std::array<double, 3> pos = get_simplex_position(id);
            double dx = pos[0] - center[0];
            double dy = pos[1] - center[1];
            double dz = pos[2] - center[2];
            double dist_sq = dx*dx + dy*dy + dz*dz;
            if (dist_sq <= radius_sq) {
                results.simplex_ids.push_back(id);
            }
        }
    } else {
        query_sphere_recursive(node->left.get(), center, radius_sq, results);
        query_sphere_recursive(node->right.get(), center, radius_sq, results);
    }
}

std::vector<SimplexID> BVH::nearest_neighbors(const std::array<double, 3>& point,
                                             size_t k) const {
    // Priority queue for k nearest neighbors (max-heap)
    using Pair = std::pair<double, SimplexID>;
    std::priority_queue<Pair> pq;

    std::function<void(const BVHNode*)> search = [&](const BVHNode* node) {
        if (!node) {
            return;
        }

        if (!node->bbox.contains(point)) {
            return;
        }

        if (node->is_leaf()) {
            for (SimplexID id : node->simplices) {
                std::array<double, 3> pos = get_simplex_position(id);
                double dx = pos[0] - point[0];
                double dy = pos[1] - point[1];
                double dz = pos[2] - point[2];
                double dist_sq = dx*dx + dy*dy + dz*dz;

                if (pq.size() < k) {
                    pq.push({dist_sq, id});
                } else if (dist_sq < pq.top().first) {
                    pq.pop();
                    pq.push({dist_sq, id});
                }
            }
        } else {
            search(node->left.get());
            search(node->right.get());
        }
    };

    if (root_) {
        search(root_.get());
    }

    // Extract results
    std::vector<SimplexID> results;
    while (!pq.empty()) {
        results.push_back(pq.top().second);
        pq.pop();
    }
    std::reverse(results.begin(), results.end());
    return results;
}

std::vector<SimplexID> BVH::ray_intersect(const std::array<double, 3>& origin,
                                         const std::array<double, 3>& direction) const {
    std::vector<SimplexID> results;

    std::function<void(const BVHNode*)> search = [&](const BVHNode* node) {
        if (!node) {
            return;
        }

        // Ray-box intersection test (simplified)
        std::array<double, 3> t_min, t_max;
        for (int i = 0; i < 3; ++i) {
            double inv_d = 1.0 / direction[i];
            t_min[i] = (node->bbox.min[i] - origin[i]) * inv_d;
            t_max[i] = (node->bbox.max[i] - origin[i]) * inv_d;
            if (t_min[i] > t_max[i]) {
                std::swap(t_min[i], t_max[i]);
            }
        }

        double t_enter = std::max({t_min[0], t_min[1], t_min[2]});
        double t_exit = std::min({t_max[0], t_max[1], t_max[2]});

        if (t_exit < 0 || t_enter > t_exit) {
            return;
        }

        if (node->is_leaf()) {
            for (SimplexID id : node->simplices) {
                results.push_back(id);
            }
        } else {
            search(node->left.get());
            search(node->right.get());
        }
    };

    if (root_) {
        search(root_.get());
    }

    return results;
}

void BVH::update_simplex(SimplexID id, const std::array<double, 3>& new_position) {
    needs_rebuild_ = true;
    // Mark for incremental update (placeholder)
}

void BVH::remove_simplex(SimplexID id) {
    needs_rebuild_ = true;
    // Mark for incremental update (placeholder)
}

size_t BVH::get_node_count() const {
    return count_nodes(root_.get());
}

size_t BVH::count_nodes(const BVHNode* node) const {
    if (!node) {
        return 0;
    }
    return 1 + count_nodes(node->left.get()) + count_nodes(node->right.get());
}

size_t BVH::get_depth() const {
    return get_max_depth(root_.get(), 0);
}

size_t BVH::get_max_depth(const BVHNode* node, size_t current) const {
    if (!node) {
        return current;
    }
    return std::max(get_max_depth(node->left.get(), current + 1),
                    get_max_depth(node->right.get(), current + 1));
}

size_t BVH::get_leaf_count() const {
    return count_leaves(root_.get());
}

size_t BVH::count_leaves(const BVHNode* node) const {
    if (!node) {
        return 0;
    }
    if (node->is_leaf()) {
        return 1;
    }
    return count_leaves(node->left.get()) + count_leaves(node->right.get());
}

size_t BVH::get_simplex_count() const {
    if (!root_) {
        return 0;
    }
    std::function<size_t(const BVHNode*)> count = [&](const BVHNode* node) -> size_t {
        if (!node) {
            return 0;
        }
        if (node->is_leaf()) {
            return node->simplices.size();
        }
        return count(node->left.get()) + count(node->right.get());
    };
    return count(root_.get());
}

double BVH::get_average_leaf_size() const {
    size_t leaf_count = get_leaf_count();
    if (leaf_count == 0) {
        return 0.0;
    }
    return static_cast<double>(get_simplex_count()) / leaf_count;
}

BVHQueryResult BVH::get_statistics() const {
    BVHQueryResult stats;
    stats.nodes_visited = get_node_count();
    return stats;
}

void BVH::set_build_strategy(BVHBuildStrategy strategy) {
    build_strategy_ = strategy;
    needs_rebuild_ = true;
}

BVHBuildStrategy BVH::get_build_strategy() const {
    return build_strategy_;
}

void BVH::set_max_leaf_size(size_t size) {
    max_leaf_size_ = size;
    needs_rebuild_ = true;
}

size_t BVH::get_max_leaf_size() const {
    return max_leaf_size_;
}

void BVH::set_max_depth(size_t depth) {
    max_depth_ = depth;
    needs_rebuild_ = true;
}

size_t BVH::get_max_depth() const {
    return max_depth_;
}

bool BVH::validate() const {
    return validate_recursive(root_.get());
}

bool BVH::validate_recursive(const BVHNode* node) const {
    if (!node) {
        return true;
    }

    // Check leaf nodes have no children
    if (node->is_leaf()) {
        if (node->left || node->right) {
            return false;
        }
    } else {
        // Check internal nodes have children
        if (!node->left || !node->right) {
            return false;
        }
    }

    return validate_recursive(node->left.get()) && validate_recursive(node->right.get());
}

void BVH::print_tree() const {
    std::cout << "BVH Tree Structure:" << std::endl;
    print_tree_recursive(root_.get(), 0);
}

void BVH::print_tree_recursive(const BVHNode* node, int depth) const {
    if (!node) {
        return;
    }

    for (int i = 0; i < depth; ++i) {
        std::cout << "  ";
    }

    if (node->is_leaf()) {
        std::cout << "Leaf: " << node->simplices.size() << " simplices" << std::endl;
    } else {
        std::cout << "Internal" << std::endl;
        print_tree_recursive(node->left.get(), depth + 1);
        print_tree_recursive(node->right.get(), depth + 1);
    }
}

BoundingBox BVH::compute_bounding_box(SimplexID id) const {
    BoundingBox bbox;
    
    // Get vertices of simplex
    std::vector<VertexID> vertices;
    if (complex_) {
        const auto& simplex = complex_->get_simplex(id);
        for (VertexID vid : simplex.vertices) {
            const auto& vertex = complex_->get_vertex(vid);
            bbox.expand({vertex.x, vertex.y, vertex.z});
        }
    } else if (labeled_complex_) {
        const auto& simplex = labeled_complex_->get_simplex(id);
        for (VertexID vid : simplex.vertices) {
            const auto& vertex = labeled_complex_->get_vertex(vid);
            bbox.expand({vertex.x, vertex.y, vertex.z});
        }
    }

    bbox.simplex_id = id;
    return bbox;
}

std::array<double, 3> BVH::get_simplex_position(SimplexID id) const {
    std::array<double, 3> pos{0.0, 0.0, 0.0};
    
    std::vector<VertexID> vertices;
    if (complex_) {
        const auto& simplex = complex_->get_simplex(id);
        for (VertexID vid : simplex.vertices) {
            const auto& vertex = complex_->get_vertex(vid);
            pos[0] += vertex.x;
            pos[1] += vertex.y;
            pos[2] += vertex.z;
        }
    } else if (labeled_complex_) {
        const auto& simplex = labeled_complex_->get_simplex(id);
        for (VertexID vid : simplex.vertices) {
            const auto& vertex = labeled_complex_->get_vertex(vid);
            pos[0] += vertex.x;
            pos[1] += vertex.y;
            pos[2] += vertex.z;
        }
    }

    if (!vertices.empty()) {
        pos[0] /= vertices.size();
        pos[1] /= vertices.size();
        pos[2] /= vertices.size();
    }

    return pos;
}

} // namespace cebu
