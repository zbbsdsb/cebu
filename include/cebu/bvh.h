#pragma once

#include "cebu/common.h"
#include "cebu/vertex_geometry.h"
#include <vector>
#include <memory>
#include <functional>
#include <optional>

namespace cebu {

/**
 * @brief BVH build strategy
 */
enum class BVHBuildStrategy {
    MEDIAN_SPLIT,      // Split at median along longest axis
    SAH,               // Surface Area Heuristic
    MIDPOINT_SPLIT,    // Split at midpoint
    HLBVH,             // Hybrid Linear BVH (advanced)
    EQUAL_COUNTS       // Split to equal number of primitives
};

/**
 * @brief BVH node structure
 */
struct BVHNode {
    BoundingBox bbox;                   // Bounding box of the node
    size_t left_child;                  // Index of left child (or INVALID if none)
    size_t right_child;                 // Index of right child (or INVALID if none)
    std::vector<SimplexID> simplices;   // Simplices stored in leaf nodes
    bool is_leaf;                       // Whether this is a leaf node
    int depth;                          // Depth in the tree

    BVHNode()
        : left_child(SIZE_MAX),
          right_child(SIZE_MAX),
          is_leaf(false),
          depth(0) {}

    bool has_left_child() const {
        return left_child != SIZE_MAX;
    }

    bool has_right_child() const {
        return right_child != SIZE_MAX;
    }

    bool is_leaf_node() const {
        return is_leaf;
    }

    size_t simplex_count() const {
        return simplices.size();
    }
};

/**
 * @brief BVH tree for spatial indexing of simplices
 *
 * Bounding Volume Hierarchy (BVH) is a space-partitioning data structure
 * that organizes geometric objects in a tree structure to accelerate
 * spatial queries like ray intersection, collision detection, etc.
 *
 * This implementation is optimized for:
 * - Fast spatial queries (range, nearest neighbor)
 * - Incremental updates
 * - Memory efficiency
 * - Cache-friendly traversal
 */
class BVHTree {
public:
    /**
     * @brief Constructor
     * @param strategy Build strategy for the BVH
     * @param max_simplices_per_node Maximum simplices per leaf node
     * @param max_depth Maximum tree depth
     */
    explicit BVHTree(
        BVHBuildStrategy strategy = BVHBuildStrategy::SAH,
        size_t max_simplices_per_node = 8,
        size_t max_depth = 20
    );

    /**
     * @brief Build the BVH from a list of simplices
     * @param simplices List of simplex IDs to index
     * @param vertex_geometry Vertex position data
     * @param get_simplex_vertices Function to get vertices of a simplex
     */
    void build(
        const std::vector<SimplexID>& simplices,
        const VertexGeometry& vertex_geometry,
        std::function<std::vector<VertexID>(SimplexID)> get_simplex_vertices
    );

    /**
     * @brief Rebuild the BVH (useful after modifications)
     */
    void rebuild();

    /**
     * @brief Add a simplex to the BVH (incremental update)
     * @param simplex_id Simplex ID to add
     */
    void add_simplex(SimplexID simplex_id);

    /**
     * @brief Remove a simplex from the BVH
     * @param simplex_id Simplex ID to remove
     */
    void remove_simplex(SimplexID simplex_id);

    /**
     * @brief Clear the BVH
     */
    void clear();

    /**
     * @brief Query simplices that intersect with a bounding box
     * @param bbox Bounding box to query
     * @return List of simplex IDs intersecting the bbox
     */
    std::vector<SimplexID> query_range(const BoundingBox& bbox) const;

    /**
     * @brief Query the k nearest simplices to a point
     * @param point Query point
     * @param k Number of nearest neighbors to find
     * @return List of simplex IDs, sorted by distance
     */
    std::vector<SimplexID> query_nearest(const Point3D& point, size_t k = 1) const;

    /**
     * @brief Query simplices that intersect with a ray
     * @param origin Ray origin
     * @param direction Ray direction (normalized)
     * @return List of simplex IDs intersecting the ray
     */
    std::vector<SimplexID> query_ray(const Point3D& origin, const Point3D& direction) const;

    /**
     * @brief Check if a simplex intersects with a point
     * @param point Query point
     * @param simplex_id Simplex ID
     * @return true if the simplex contains the point
     */
    bool simplex_contains_point(const Point3D& point, SimplexID simplex_id) const;

    /**
     * @brief Get the bounding box of the entire BVH
     */
    BoundingBox get_bounding_box() const;

    /**
     * @brief Get the number of nodes in the tree
     */
    size_t node_count() const { return nodes_.size(); }

    /**
     * @brief Get the number of simplices indexed
     */
    size_t simplex_count() const { return simplex_ids_.size(); }

    /**
     * @brief Get the maximum depth of the tree
     */
    size_t max_depth() const { return max_depth_; }

    /**
     * @brief Check if the BVH is built
     */
    bool is_built() const { return !nodes_.empty(); }

    /**
     * @brief Get statistics about the BVH structure
     */
    struct Statistics {
        size_t total_nodes;
        size_t leaf_nodes;
        size_t internal_nodes;
        size_t max_tree_depth;
        size_t avg_simplices_per_leaf;
        size_t max_simplices_per_leaf;
    };
    Statistics get_statistics() const;

private:
    // Tree data
    std::vector<BVHNode> nodes_;
    std::unordered_map<SimplexID, size_t> simplex_to_leaves_;  // Simplex -> leaf node index
    std::vector<SimplexID> simplex_ids_;

    // Build parameters
    BVHBuildStrategy strategy_;
    size_t max_simplices_per_node_;
    size_t max_depth_;

    // References for building
    VertexGeometry* vertex_geometry_;
    std::function<std::vector<VertexID>(SimplexID)> get_simplex_vertices_;

    /**
     * @brief Create a new node
     * @return Index of the new node
     */
    size_t create_node();

    /**
     * @brief Build a subtree recursively
     * @param simplex_ids List of simplex IDs for this subtree
     * @param depth Current depth
     * @return Index of the root node
     */
    size_t build_recursive(const std::vector<SimplexID>& simplex_ids, int depth);

    /**
     * @brief Build subtree using median split
     */
    size_t build_median_split(const std::vector<SimplexID>& simplex_ids, int depth);

    /**
     * @brief Build subtree using Surface Area Heuristic
     */
    size_t build_sah(const std::vector<SimplexID>& simplex_ids, int depth);

    /**
     * @brief Build subtree using midpoint split
     */
    size_t build_midpoint_split(const std::vector<SimplexID>& simplex_ids, int depth);

    /**
     * @brief Build subtree using equal counts
     */
    size_t build_equal_counts(const std::vector<SimplexID>& simplex_ids, int depth);

    /**
     * @brief Compute bounding box for a list of simplices
     */
    BoundingBox compute_bbox(const std::vector<SimplexID>& simplex_ids) const;

    /**
     * @brief Get the axis with the largest extent
     */
    int get_longest_axis(const BoundingBox& bbox) const;

    /**
     * @brief Partition simplices by an axis
     */
    void partition_by_axis(
        const std::vector<SimplexID>& simplex_ids,
        int axis,
        float split_point,
        std::vector<SimplexID>& left,
        std::vector<SimplexID>& right
    ) const;

    /**
     * @brief Get the centroid of a simplex
     */
    Point3D get_simplex_centroid(SimplexID simplex_id) const;

    /**
     * @brief Recursive range query
     */
    void query_range_recursive(size_t node_idx, const BoundingBox& bbox,
                               std::vector<SimplexID>& results) const;

    /**
     * @brief Recursive nearest neighbor query using priority queue
     */
    void query_nearest_recursive(size_t node_idx, const Point3D& point,
                                  size_t k, std::vector<std::pair<float, SimplexID>>& candidates) const;

    /**
     * @brief Recursive ray query
     */
    void query_ray_recursive(size_t node_idx, const Point3D& origin,
                              const Point3D& direction, std::vector<SimplexID>& results) const;

    /**
     * @brief Check if ray intersects with bounding box
     */
    bool ray_intersects_bbox(const Point3D& origin, const Point3D& direction,
                             const BoundingBox& bbox) const;

    /**
     * @brief Update statistics recursively
     */
    void compute_statistics_recursive(size_t node_idx, Statistics& stats, int current_depth) const;
};

} // namespace cebu
