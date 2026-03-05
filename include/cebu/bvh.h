#ifndef CEBU_BVH_H
#define CEBU_BVH_H

#include "simplicial_complex.h"
#include "simplicial_complex_labeled.h"
#include <vector>
#include <memory>
#include <array>
#include <limits>
#include <functional>

namespace cebu {

/**
 * @brief 3D axis-aligned bounding box
 */
struct BoundingBox {
    std::array<double, 3> min;
    std::array<double, 3> max;
    SimplexID simplex_id;

    BoundingBox() : min{{std::numeric_limits<double>::max(),
                         std::numeric_limits<double>::max(),
                         std::numeric_limits<double>::max()}},
                    max{{std::numeric_limits<double>::lowest(),
                         std::numeric_limits<double>::lowest(),
                         std::numeric_limits<double>::lowest()}},
                    simplex_id(0) {}

    BoundingBox(const std::array<double, 3>& min_val,
                const std::array<double, 3>& max_val,
                SimplexID id)
        : min(min_val), max(max_val), simplex_id(id) {}

    /**
     * @brief Expand bounding box to include point
     */
    void expand(const std::array<double, 3>& point) {
        for (int i = 0; i < 3; ++i) {
            min[i] = std::min(min[i], point[i]);
            max[i] = std::max(max[i], point[i]);
        }
    }

    /**
     * @brief Merge with another bounding box
     */
    void merge(const BoundingBox& other) {
        for (int i = 0; i < 3; ++i) {
            min[i] = std::min(min[i], other.min[i]);
            max[i] = std::max(max[i], other.max[i]);
        }
    }

    /**
     * @brief Calculate surface area (for SAH)
     */
    double surface_area() const {
        double dx = max[0] - min[0];
        double dy = max[1] - min[1];
        double dz = max[2] - min[2];
        return 2.0 * (dx * dy + dy * dz + dz * dx);
    }

    /**
     * @brief Check if point is inside bounding box
     */
    bool contains(const std::array<double, 3>& point) const {
        return point[0] >= min[0] && point[0] <= max[0] &&
               point[1] >= min[1] && point[1] <= max[1] &&
               point[2] >= min[2] && point[2] <= max[2];
    }

    /**
     * @brief Check if intersects with another bounding box
     */
    bool intersects(const BoundingBox& other) const {
        return !(max[0] < other.min[0] || min[0] > other.max[0] ||
                 max[1] < other.min[1] || min[1] > other.max[1] ||
                 max[2] < other.min[2] || min[2] > other.max[2]);
    }

    /**
     * @brief Calculate center point
     */
    std::array<double, 3> center() const {
        return {(min[0] + max[0]) / 2.0,
                (min[1] + max[1]) / 2.0,
                (min[2] + max[2]) / 2.0};
    }
};

/**
 * @brief BVH node type
 */
enum class BVHNodeType {
    LEAF,       // Leaf node containing simplices
    INTERNAL    // Internal node with children
};

/**
 * @brief BVH tree node
 */
struct BVHNode {
    BoundingBox bbox;
    BVHNodeType type;
    std::unique_ptr<BVHNode> left;
    std::unique_ptr<BVHNode> right;
    std::vector<SimplexID> simplices;  // For leaf nodes

    BVHNode() : type(BVHNodeType::LEAF) {}

    bool is_leaf() const { return type == BVHNodeType::LEAF; }
};

/**
 * @brief BVH build strategy
 */
enum class BVHBuildStrategy {
    MIDPOINT,       // Split at midpoint
    MEDIAN,         // Split at median (balanced tree)
    SAH,            // Surface Area Heuristic (optimal)
    HILBERT         // Hilbert curve ordering
};

/**
 * @brief BVH query result
 */
struct BVHQueryResult {
    std::vector<SimplexID> simplex_ids;
    size_t nodes_visited;
    double query_time_ms;

    BVHQueryResult() : nodes_visited(0), query_time_ms(0.0) {}
};

/**
 * @brief High-performance Bounding Volume Hierarchy
 * 
 * Provides O(log n) spatial queries for simplicial complexes.
 * Supports dynamic rebuilding and adaptive optimization.
 */
class BVH {
public:
    BVH();
    explicit BVH(const SimplicialComplex& complex);
    explicit BVH(const SimplicialComplexLabeled<>& complex);
    
    ~BVH();

    // Build BVH from simplicial complex
    void build(const SimplicialComplex& complex);
    void build(const SimplicialComplexLabeled<>& complex);
    
    // Rebuild BVH (after modifications)
    void rebuild();
    void rebuild(BVHBuildStrategy strategy);

    // Point queries
    bool contains(const std::array<double, 3>& point) const;
    std::vector<SimplexID> query_point(const std::array<double, 3>& point) const;
    
    // Range queries
    BVHQueryResult query_range(const BoundingBox& range) const;
    BVHQueryResult query_sphere(const std::array<double, 3>& center, double radius) const;
    
    // Nearest neighbor
    std::vector<SimplexID> nearest_neighbors(const std::array<double, 3>& point,
                                             size_t k) const;
    
    // Ray intersection
    std::vector<SimplexID> ray_intersect(const std::array<double, 3>& origin,
                                         const std::array<double, 3>& direction) const;

    // Update simplex
    void update_simplex(SimplexID id, const std::array<double, 3>& new_position);
    void remove_simplex(SimplexID id);
    
    // Statistics and diagnostics
    size_t get_node_count() const;
    size_t get_depth() const;
    size_t get_leaf_count() const;
    size_t get_simplex_count() const;
    double get_average_leaf_size() const;
    BVHQueryResult get_statistics() const;

    // Optimization
    void set_build_strategy(BVHBuildStrategy strategy);
    BVHBuildStrategy get_build_strategy() const;
    void set_max_leaf_size(size_t size);
    size_t get_max_leaf_size() const;
    void set_max_depth(size_t depth);
    size_t get_max_depth() const;

    // Validation
    bool validate() const;
    void print_tree() const;

private:
    std::unique_ptr<BVHNode> root_;
    const SimplicialComplex* complex_;
    const SimplicialComplexLabeled<>* labeled_complex_;
    BVHBuildStrategy build_strategy_;
    size_t max_leaf_size_;
    size_t max_depth_;
    bool needs_rebuild_;

    // Build methods
    void build_recursive(BVHNode* node, std::vector<SimplexID>& simplex_ids,
                        size_t depth);
    void build_midpoint(BVHNode* node, std::vector<SimplexID>& simplex_ids, size_t depth);
    void build_median(BVHNode* node, std::vector<SimplexID>& simplex_ids, size_t depth);
    void build_sah(BVHNode* node, std::vector<SimplexID>& simplex_ids, size_t depth);
    void build_hilbert(BVHNode* node, std::vector<SimplexID>& simplex_ids, size_t depth);

    // Query methods
    void query_point_recursive(const BVHNode* node, const std::array<double, 3>& point,
                               std::vector<SimplexID>& results) const;
    void query_range_recursive(const BVHNode* node, const BoundingBox& range,
                               BVHQueryResult& results) const;
    void query_sphere_recursive(const BVHNode* node, const std::array<double, 3>& center,
                                double radius_sq, BVHQueryResult& results) const;

    // Utility methods
    BoundingBox compute_bounding_box(SimplexID id) const;
    std::array<double, 3> get_simplex_position(SimplexID id) const;
    size_t count_nodes(const BVHNode* node) const;
    size_t get_max_depth(const BVHNode* node, size_t current) const;
    size_t count_leaves(const BVHNode* node) const;
    bool validate_recursive(const BVHNode* node) const;
    void print_tree_recursive(const BVHNode* node, int depth) const;
};

} // namespace cebu

#endif // CEBU_BVH_H
