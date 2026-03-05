#include "cebu/octree.h"
#include <algorithm>
#include <queue>
#include <cmath>
#include <iostream>
#include <limits>

namespace cebu {

// ============================================================================
// Octree Implementation
// ============================================================================

Octree::Octree()
    : complex_(nullptr)
    , labeled_complex_(nullptr)
    , max_simplices_per_node_(32)
    , max_depth_(16)
    , current_lod_level_(0) {}

Octree::Octree(const SimplicialComplex& complex)
    : complex_(&complex)
    , labeled_complex_(nullptr)
    , max_simplices_per_node_(32)
    , max_depth_(16)
    , current_lod_level_(0) {
    build(complex);
}

Octree::Octree(const SimplicialComplexLabeled<>& complex)
    : complex_(nullptr)
    , labeled_complex_(&complex)
    , max_simplices_per_node_(32)
    , max_depth_(16)
    , current_lod_level_(0) {
    build(complex);
}

Octree::~Octree() = default;

void Octree::build(const SimplicialComplex& complex) {
    complex_ = &complex;
    labeled_complex_ = nullptr;

    // Compute bounding box
    std::array<double, 3> min, max;
    compute_bounds(complex, min, max);

    // Create root node
    std::array<double, 3> center = {
        (min[0] + max[0]) / 2.0,
        (min[1] + max[1]) / 2.0,
        (min[2] + max[2]) / 2.0
    };
    double extent = std::max({max[0] - min[0], max[1] - min[1], max[2] - min[2]}) / 2.0;
    root_ = std::make_unique<OctreeNode>(center, extent);

    // Collect all simplex IDs
    std::vector<SimplexID> simplex_ids;
    simplex_ids.reserve(complex.simplex_count());

    for (const auto& [id, simplex] : complex.get_all_simplices()) {
        simplex_ids.push_back(id);
    }

    // Build octree recursively
    build_recursive(root_.get(), simplex_ids, 0);
}

void Octree::build(const SimplicialComplexLabeled<>& complex) {
    labeled_complex_ = &complex;
    complex_ = nullptr;

    // Compute bounding box
    std::array<double, 3> min, max;
    compute_bounds(complex, min, max);

    // Create root node
    std::array<double, 3> center = {
        (min[0] + max[0]) / 2.0,
        (min[1] + max[1]) / 2.0,
        (min[2] + max[2]) / 2.0
    };
    double extent = std::max({max[0] - min[0], max[1] - min[1], max[2] - min[2]}) / 2.0;
    root_ = std::make_unique<OctreeNode>(center, extent);

    // Collect all simplex IDs
    std::vector<SimplexID> simplex_ids;
    simplex_ids.reserve(complex.simplex_count());

    for (const auto& [id, simplex] : complex.get_all_simplices()) {
        simplex_ids.push_back(id);
    }

    // Build octree recursively
    build_recursive(root_.get(), simplex_ids, 0);
}

void Octree::rebuild() {
    if (complex_) {
        build(*complex_);
    } else if (labeled_complex_) {
        build(*labeled_complex_);
    }
}

void Octree::build_recursive(OctreeNode* node, const std::vector<SimplexID>& simplex_ids,
                             size_t depth) {
    if (simplex_ids.empty() || depth >= max_depth_) {
        node->simplices = simplex_ids;
        return;
    }

    node->simplices = simplex_ids;

    // Check if subdivision is needed
    if (simplex_ids.size() > max_simplices_per_node_ && depth < max_depth_) {
        subdivide(node);

        // Distribute simplices to children
        for (SimplexID id : simplex_ids) {
            std::array<double, 3> pos = get_simplex_position(id);
            int child_idx = get_child_index(node, pos);
            if (child_idx >= 0) {
                node->children[child_idx]->simplices.push_back(id);
            } else {
                // Keep in parent (boundary case)
            }
        }

        // Recursively build children
        for (auto& child : node->children) {
            if (!child->simplices.empty()) {
                build_recursive(child.get(), child->simplices, depth + 1);
            }
        }

        // Clear parent's simplices
        node->simplices.clear();
        node->is_leaf = false;
    }
}

void Octree::subdivide(OctreeNode* node) {
    double child_extent = node->extent / 2.0;

    for (int i = 0; i < 8; ++i) {
        std::array<double, 3> child_center = node->center;
        child_center[0] += (i & 1) ? child_extent : -child_extent;
        child_center[1] += (i & 2) ? child_extent : -child_extent;
        child_center[2] += (i & 4) ? child_extent : -child_extent;
        node->children[i] = std::make_unique<OctreeNode>(child_center, child_extent);
    }
}

int Octree::get_child_index(const OctreeNode* node, const std::array<double, 3>& point) const {
    int idx = 0;
    for (int i = 0; i < 3; ++i) {
        if (point[i] > node->center[i]) {
            idx |= (1 << i);
        } else if (point[i] == node->center[i]) {
            return -1; // On boundary
        }
    }
    return idx;
}

bool Octree::contains(const std::array<double, 3>& point) const {
    return !query_point(point).empty();
}

std::vector<SimplexID> Octree::query_point(const std::array<double, 3>& point) const {
    std::vector<SimplexID> results;
    size_t depth = 0;
    if (root_) {
        query_point_recursive(root_.get(), point, results, depth);
    }
    return results;
}

void Octree::query_point_recursive(const OctreeNode* node, const std::array<double, 3>& point,
                                   std::vector<SimplexID>& results, size_t& depth) const {
    if (!node || !node->contains(point)) {
        return;
    }

    depth++;

    if (node->is_leaf) {
        results.insert(results.end(), node->simplices.begin(), node->simplices.end());
    } else {
        int child_idx = get_child_index(node, point);
        if (child_idx >= 0) {
            query_point_recursive(node->children[child_idx].get(), point, results, depth);
        } else {
            // Check all children (boundary case)
            for (const auto& child : node->children) {
                query_point_recursive(child.get(), point, results, depth);
            }
        }
    }
}

OctreeQueryResult Octree::query_range(const std::array<double, 3>& min,
                                      const std::array<double, 3>& max) const {
    OctreeQueryResult result;
    if (root_) {
        query_range_recursive(root_.get(), min, max, result);
    }
    return result;
}

void Octree::query_range_recursive(const OctreeNode* node, const std::array<double, 3>& min,
                                   const std::array<double, 3>& max,
                                   OctreeQueryResult& results) const {
    if (!node) {
        return;
    }

    results.nodes_visited++;
    results.depth = std::max(results.depth, static_cast<size_t>(0));

    // Check if node bounds intersect query range
    std::array<double, 3> node_min, node_max;
    node->get_bounds(node_min, node_max);

    if (node_max[0] < min[0] || node_min[0] > max[0] ||
        node_max[1] < min[1] || node_min[1] > max[1] ||
        node_max[2] < min[2] || node_min[2] > max[2]) {
        return;
    }

    if (node->is_leaf) {
        // Check each simplex
        for (SimplexID id : node->simplices) {
            std::array<double, 3> pos = get_simplex_position(id);
            if (pos[0] >= min[0] && pos[0] <= max[0] &&
                pos[1] >= min[1] && pos[1] <= max[1] &&
                pos[2] >= min[2] && pos[2] <= max[2]) {
                results.simplex_ids.push_back(id);
            }
        }
    } else {
        for (const auto& child : node->children) {
            query_range_recursive(child.get(), min, max, results);
        }
    }
}

OctreeQueryResult Octree::query_sphere(const std::array<double, 3>& center, double radius) const {
    OctreeQueryResult result;
    double radius_sq = radius * radius;
    if (root_) {
        query_sphere_recursive(root_.get(), center, radius_sq, result);
    }
    return result;
}

void Octree::query_sphere_recursive(const OctreeNode* node, const std::array<double, 3>& center,
                                    double radius_sq, OctreeQueryResult& results) const {
    if (!node) {
        return;
    }

    results.nodes_visited++;

    // Quick rejection: check if node intersects sphere
    std::array<double, 3> node_min, node_max;
    node->get_bounds(node_min, node_max);

    double dist_sq = 0.0;
    for (int i = 0; i < 3; ++i) {
        if (center[i] < node_min[i]) {
            dist_sq += (center[i] - node_min[i]) * (center[i] - node_min[i]);
        } else if (center[i] > node_max[i]) {
            dist_sq += (center[i] - node_max[i]) * (center[i] - node_max[i]);
        }
    }

    if (dist_sq > radius_sq) {
        return;
    }

    if (node->is_leaf) {
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
        for (const auto& child : node->children) {
            query_sphere_recursive(child.get(), center, radius_sq, results);
        }
    }
}

std::vector<SimplexID> Octree::nearest_neighbors(const std::array<double, 3>& point,
                                                 size_t k) const {
    using Pair = std::pair<double, SimplexID>;
    std::priority_queue<Pair> pq;

    std::function<void(const OctreeNode*)> search = [&](const OctreeNode* node) {
        if (!node || !node->contains(point)) {
            return;
        }

        if (node->is_leaf) {
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
            for (const auto& child : node->children) {
                search(child.get());
            }
        }
    };

    if (root_) {
        search(root_.get());
    }

    std::vector<SimplexID> results;
    while (!pq.empty()) {
        results.push_back(pq.top().second);
        pq.pop();
    }
    std::reverse(results.begin(), results.end());
    return results;
}

std::vector<SimplexID> Octree::query_frustum(const std::array<double, 16>& view_proj) const {
    // Simplified frustum culling (placeholder)
    // In production, would use full frustum plane equations
    std::vector<SimplexID> results;

    std::function<void(const OctreeNode*)> traverse = [&](const OctreeNode* node) {
        if (!node) {
            return;
        }

        if (node->is_leaf) {
            for (SimplexID id : node->simplices) {
                results.push_back(id);
            }
        } else {
            for (const auto& child : node->children) {
                traverse(child.get());
            }
        }
    };

    if (root_) {
        traverse(root_.get());
    }

    return results;
}

void Octree::insert_simplex(SimplexID id) {
    // For incremental updates, would rebuild or use insert algorithm
    rebuild();
}

void Octree::remove_simplex(SimplexID id) {
    // For incremental updates, would rebuild or use delete algorithm
    rebuild();
}

void Octree::update_simplex(SimplexID id) {
    // For incremental updates, would rebuild or use update algorithm
    rebuild();
}

void Octree::set_lod_level(int level) {
    current_lod_level_ = std::max(0, level);
}

int Octree::get_lod_level() const {
    return current_lod_level_;
}

std::vector<SimplexID> Octree::get_lod_simplices(int lod_level) const {
    std::vector<SimplexID> results;

    std::function<void(const OctreeNode*, int)> collect = [&](const OctreeNode* node, int depth) {
        if (!node) {
            return;
        }

        if (depth >= lod_level || node->is_leaf) {
            results.insert(results.end(), node->simplices.begin(), node->simplices.end());
        } else {
            for (const auto& child : node->children) {
                collect(child.get(), depth + 1);
            }
        }
    };

    if (root_) {
        collect(root_.get(), 0);
    }

    return results;
}

size_t Octree::get_node_count() const {
    return count_nodes(root_.get());
}

size_t Octree::count_nodes(const OctreeNode* node) const {
    if (!node) {
        return 0;
    }
    size_t count = 1;
    if (!node->is_leaf) {
        for (const auto& child : node->children) {
            count += count_nodes(child.get());
        }
    }
    return count;
}

size_t Octree::get_depth() const {
    return get_max_depth(root_.get(), 0);
}

size_t Octree::get_max_depth(const OctreeNode* node, size_t current) const {
    if (!node || node->is_leaf) {
        return current;
    }
    size_t max_depth = current;
    for (const auto& child : node->children) {
        max_depth = std::max(max_depth, get_max_depth(child.get(), current + 1));
    }
    return max_depth;
}

size_t Octree::get_leaf_count() const {
    return count_leaves(root_.get());
}

size_t Octree::count_leaves(const OctreeNode* node) const {
    if (!node) {
        return 0;
    }
    if (node->is_leaf) {
        return 1;
    }
    size_t count = 0;
    for (const auto& child : node->children) {
        count += count_leaves(child.get());
    }
    return count;
}

size_t Octree::get_simplex_count() const {
    return count_simplices(root_.get());
}

size_t Octree::count_simplices(const OctreeNode* node) const {
    if (!node) {
        return 0;
    }
    if (node->is_leaf) {
        return node->simplices.size();
    }
    size_t count = 0;
    for (const auto& child : node->children) {
        count += count_simplices(child.get());
    }
    return count;
}

size_t Octree::get_max_simplices_per_node() const {
    std::function<size_t(const OctreeNode*)> max_count = [&](const OctreeNode* node) -> size_t {
        if (!node) {
            return 0;
        }
        if (node->is_leaf) {
            return node->simplices.size();
        }
        size_t max_s = 0;
        for (const auto& child : node->children) {
            max_s = std::max(max_s, max_count(child.get()));
        }
        return max_s;
    };
    return max_count(root_.get());
}

OctreeQueryResult Octree::get_statistics() const {
    OctreeQueryResult stats;
    stats.nodes_visited = get_node_count();
    stats.depth = get_depth();
    return stats;
}

void Octree::set_max_simplices_per_node(size_t max) {
    max_simplices_per_node_ = max;
}

size_t Octree::get_max_simplices_per_node_param() const {
    return max_simplices_per_node_;
}

void Octree::set_max_depth(size_t depth) {
    max_depth_ = depth;
}

size_t Octree::get_max_depth_param() const {
    return max_depth_;
}

bool Octree::validate() const {
    return validate_recursive(root_.get());
}

bool Octree::validate_recursive(const OctreeNode* node) const {
    if (!node) {
        return true;
    }

    if (!node->is_leaf) {
        // Check all children exist
        for (const auto& child : node->children) {
            if (!child) {
                return false;
            }
            if (!validate_recursive(child.get())) {
                return false;
            }
        }
    }

    return true;
}

void Octree::print_tree() const {
    std::cout << "Octree Structure:" << std::endl;
    print_tree_recursive(root_.get(), 0);
}

void Octree::print_tree_recursive(const OctreeNode* node, int depth) const {
    if (!node) {
        return;
    }

    for (int i = 0; i < depth; ++i) {
        std::cout << "  ";
    }

    if (node->is_leaf) {
        std::cout << "Leaf: " << node->simplices.size() << " simplices" << std::endl;
    } else {
        std::cout << "Internal" << std::endl;
        for (const auto& child : node->children) {
            print_tree_recursive(child.get(), depth + 1);
        }
    }
}

std::array<double, 3> Octree::get_simplex_position(SimplexID id) const {
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

void Octree::compute_bounds(const SimplicialComplex& complex,
                            std::array<double, 3>& min, std::array<double, 3>& max) const {
    min = {std::numeric_limits<double>::max(),
           std::numeric_limits<double>::max(),
           std::numeric_limits<double>::max()};
    max = {std::numeric_limits<double>::lowest(),
           std::numeric_limits<double>::lowest(),
           std::numeric_limits<double>::lowest()};

    for (const auto& [vid, vertex] : complex.get_all_vertices()) {
        min[0] = std::min(min[0], vertex.x);
        min[1] = std::min(min[1], vertex.y);
        min[2] = std::min(min[2], vertex.z);
        max[0] = std::max(max[0], vertex.x);
        max[1] = std::max(max[1], vertex.y);
        max[2] = std::max(max[2], vertex.z);
    }
}

void Octree::compute_bounds(const SimplicialComplexLabeled<>& complex,
                            std::array<double, 3>& min, std::array<double, 3>& max) const {
    min = {std::numeric_limits<double>::max(),
           std::numeric_limits<double>::max(),
           std::numeric_limits<double>::max()};
    max = {std::numeric_limits<double>::lowest(),
           std::numeric_limits<double>::lowest(),
           std::numeric_limits<double>::lowest()};

    for (const auto& [vid, vertex] : complex.get_all_vertices()) {
        min[0] = std::min(min[0], vertex.x);
        min[1] = std::min(min[1], vertex.y);
        min[2] = std::min(min[2], vertex.z);
        max[0] = std::max(max[0], vertex.x);
        max[1] = std::max(max[1], vertex.y);
        max[2] = std::max(max[2], vertex.z);
    }
}

} // namespace cebu
