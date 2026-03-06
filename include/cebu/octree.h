#pragma once

#include "cebu/common.h"
#include "cebu/vertex_geometry.h"
#include <vector>
#include <memory>
#include <functional>
#include <array>
#include <optional>

namespace cebu {

/**
 * @brief Octree node structure
 */
struct OctreeNode {
    BoundingBox bbox;                       // Bounding box of the node
    std::array<size_t, 8> children;          // 8 child node indices
    std::vector<SimplexID> simplices;        // Simplices in this node
    int depth;                              // Depth of the node in the tree
    bool is_leaf;                           // Whether this is a leaf node

    OctreeNode()
        : depth(0),
          is_leaf(false) {
        children.fill(SIZE_MAX);
    }

    /**
     * @brief Get the child at a specific octant
     * @param octant Octant index (0-7)
     * @return Child node index, or SIZE_MAX if no child
     */
    size_t get_child(int octant) const {
        if (octant < 0 || octant >= 8) {
            return SIZE_MAX;
        }
        return children[octant];
    }

    /**
     * @brief Set the child at a specific octant
     * @param octant Octant index (0-7)
     * @param child_index Child node index
     */
    void set_child(int octant, size_t child_index) {
        if (octant >= 0 && octant < 8) {
            children[octant] = child_index;
        }
    }

    /**
     * @brief Check if this node has a child at a specific octant
     */
    bool has_child(int octant) const {
        return get_child(octant) != SIZE_MAX;
    }

    /**
     * @brief Get the number of children
     */
    size_t child_count() const {
        size_t count = 0;
        for (size_t i = 0; i < 8; ++i) {
            if (children[i] != SIZE_MAX) {
                ++count;
            }
        }
        return count;
    }

    /**
     * @brief Get the octant for a point relative to this node's center
     * @param point The point to classify
     * @param center The center of this node
     * @return Octant index (0-7)
     */
    static int get_octant(const Point3D& point, const Point3D& center) {
        int octant = 0;
        if (point.x >= center.x) octant |= 1;
        if (point.y >= center.y) octant |= 2;
        if (point.z >= center.z) octant |= 4;
        return octant;
    }
};

/**
 * @brief Octree tree for spatial indexing of simplices
 *
 * Octree is a tree data structure for partitioning a three-dimensional space
 * by recursively subdividing it into eight octants.
 *
 * This implementation is optimized for:
 * - Uniform spatial distribution
 * - Fast range queries
 * - Dynamic updates
 * - Memory efficiency
 */
class Octree {
public:
    /**
     * @brief Constructor
     * @param max_depth Maximum depth of the tree
     * @param max_simplices_per_node Maximum simplices per leaf node
     */
    Octree(size_t max_depth = 10, size_t max_simplices_per_node = 16);

    /**
     * @brief Build the octree from a list of simplices
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
     * @brief Rebuild the octree (useful after modifications)
     */
    void rebuild();

    /**
     * @brief Add a simplex to the octree
     * @param simplex_id Simplex ID to add
     */
    void add_simplex(SimplexID simplex_id);

    /**
     * @brief Remove a simplex from the octree
     * @param simplex_id Simplex ID to remove
     */
    void remove_simplex(SimplexID simplex_id);

    /**
     * @brief Clear the octree
     */
    void clear();

    /**
     * @brief Query simplices that intersect with a bounding box
     * @param bbox Bounding box to query
     * @return List of simplex IDs intersecting the bbox
     */
    std::vector<SimplexID> query_range(const BoundingBox& bbox) const;

    /**
     * @brief Query simplices that contain a point
     * @param point Query point
     * @return List of simplex IDs containing the point
     */
    std::vector<SimplexID> query_point(const Point3D& point) const;

    /**
     * @brief Query the k nearest simplices to a point
     * @param point Query point
     * @param k Number of nearest neighbors to find
     * @return List of simplex IDs, sorted by distance
     */
    std::vector<SimplexID> query_nearest(const Point3D& point, size_t k = 1) const;

    /**
     * @brief Get the bounding box of the entire octree
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
     * @brief Check if the octree is built
     */
    bool is_built() const { return !nodes_.empty(); }

    /**
     * @brief Get statistics about the octree structure
     */
    struct Statistics {
        size_t total_nodes;
        size_t leaf_nodes;
        size_t internal_nodes;
        size_t max_tree_depth;
        size_t avg_simplices_per_leaf;
        size_t max_simplices_per_leaf;
        size_t empty_nodes;
    };
    Statistics get_statistics() const;

private:
    // Tree data
    std::vector<OctreeNode> nodes_;
    std::unordered_map<SimplexID, size_t> simplex_to_leaves_;  // Simplex -> leaf node index
    std::vector<SimplexID> simplex_ids_;

    // Build parameters
    size_t max_depth_;
    size_t max_simplices_per_node_;

    // References for building
    VertexGeometry* vertex_geometry_;
    std::function<std::vector<VertexID>(SimplexID)> get_simplex_vertices_;

    /**
     * @brief Create a new node
     * @return Index of the new node
     */
    size_t create_node(const BoundingBox& bbox, int depth);

    /**
     * @brief Build a subtree recursively
     * @param simplex_ids List of simplex IDs for this subtree
     * @param bbox Bounding box for this node
     * @param depth Current depth
     * @return Index of the node
     */
    size_t build_recursive(const std::vector<SimplexID>& simplex_ids,
                         const BoundingBox& bbox, int depth);

    /**
     * @brief Subdivide a node into 8 children
     * @param node_idx Index of the node to subdivide
     */
    void subdivide_node(size_t node_idx);

    /**
     * @brief Compute bounding box for a list of simplices
     */
    BoundingBox compute_bbox(const std::vector<SimplexID>& simplex_ids) const;

    /**
     * @brief Get the centroid of a simplex
     */
    Point3D get_simplex_centroid(SimplexID simplex_id) const;

    /**
     * @brief Determine the octant for a simplex
     * @param simplex_id Simplex ID
     * @param node_bbox Bounding box of the node
     * @return Octant index, or -1 if simplex spans multiple octants
     */
    int get_simplex_octant(SimplexID simplex_id, const BoundingBox& node_bbox) const;

    /**
     * @brief Check if a simplex fits entirely within an octant
     */
    bool simplex_fits_in_octant(SimplexID simplex_id, const BoundingBox& node_bbox, int octant) const;

    /**
     * @brief Get the bounding box for an octant
     */
    BoundingBox get_octant_bbox(const BoundingBox& node_bbox, int octant) const;

    /**
     * @brief Recursive range query
     */
    void query_range_recursive(size_t node_idx, const BoundingBox& bbox,
                             std::vector<SimplexID>& results) const;

    /**
     * @brief Recursive point query
     */
    void query_point_recursive(size_t node_idx, const Point3D& point,
                             std::vector<SimplexID>& results) const;

    /**
     * @brief Recursive nearest neighbor query
     */
    void query_nearest_recursive(size_t node_idx, const Point3D& point,
                              size_t k, std::vector<std::pair<float, SimplexID>>& candidates) const;

    /**
     * @brief Update statistics recursively
     */
    void compute_statistics_recursive(size_t node_idx, Statistics& stats, int current_depth) const;
};

} // namespace cebu
