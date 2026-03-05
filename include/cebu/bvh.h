#ifndef CEBU_BVH_H
#define CEBU_BVH_H

#include <vector>
#include <array>
#include <memory>
#include <algorithm>
#include <optional>
#include <functional>
#include <cmath>
#include <limits>
#include "cebu/simplicial_complex.h"
#include "cebu/types.h"

namespace cebu {

/**
 * @brief Bounding Volume Hierarchy (BVH) for efficient spatial queries
 *
 * BVH organizes simplices in a binary tree structure based on their
 * spatial extent, enabling efficient range queries, nearest neighbor
 * searches, and collision detection.
 */
class BVH {
public:
    /**
     * @brief Bounding box represented as [min_x, min_y, min_z, max_x, max_y, max_z]
     */
    using BoundingBox = std::array<float, 6>;

    /**
     * @brief BVH node structure
     */
    struct BVHNode {
        BoundingBox bbox;                  // Bounding box
        std::unique_ptr<BVHNode> left;      // Left child
        std::unique_ptr<BVHNode> right;     // Right child
        std::vector<SimplexID> simplices;   // Simplices in this node (leaf only)
        bool is_leaf;                       // Whether this is a leaf node
        size_t depth;                       // Depth in the tree

        BVHNode() : is_leaf(false), depth(0) {
            bbox = {std::numeric_limits<float>::max(),
                    std::numeric_limits<float>::max(),
                    std::numeric_limits<float>::max(),
                    std::numeric_limits<float>::lowest(),
                    std::numeric_limits<float>::lowest(),
                    std::numeric_limits<float>::lowest()};
        }
    };

    /**
     * @brief Construction strategy
     */
    enum class BuildStrategy {
        MEDIAN_SPLIT,    // Split by median along the longest axis
        SAH,             // Surface Area Heuristic (SAH) optimization
        MIDPOINT_SPLIT   // Split at midpoint of longest axis
    };

    /**
     * @brief Query result
     */
    struct QueryResult {
        std::vector<SimplexID> found;       // Found simplices
        size_t nodes_visited;               // Number of nodes visited
        double query_time_ms;               // Query time in milliseconds

        QueryResult() : nodes_visited(0), query_time_ms(0.0) {}
    };

    /**
     * @brief Nearest neighbor result
     */
    struct NearestResult {
        std::optional<SimplexID> simplex_id;  // Nearest simplex
        double distance;                      // Distance to nearest simplex
        size_t nodes_visited;                 // Number of nodes visited

        NearestResult() : distance(std::numeric_limits<double>::max()),
                         nodes_visited(0) {}
    };

private:
    std::unique_ptr<BVHNode> root_;
    const SimplicialComplex* complex_;
    BuildStrategy strategy_;
    size_t max_simplices_per_leaf_;
    size_t max_depth_;
    std::unordered_map<SimplexID, std::vector<float>> simplex_centers_;

    /**
     * @brief Get vertex position from complex
     */
    std::optional<std::array<float, 3>> get_vertex_position(VertexID vid) const;

    /**
     * @brief Compute bounding box for a simplex
     */
    BoundingBox compute_bounding_box(SimplexID sid) const;

    /**
     * @brief Compute bounding box for multiple simplices
     */
    BoundingBox compute_bounding_box(const std::vector<SimplexID>& simplices) const;

    /**
     * @brief Merge two bounding boxes
     */
    BoundingBox merge_bounding_boxes(const BoundingBox& a, const BoundingBox& b) const;

    /**
     * @brief Compute surface area of bounding box
     */
    float compute_surface_area(const BoundingBox& bbox) const;

    /**
     * @brief Check if bounding box intersects point
     */
    bool intersects_point(const BoundingBox& bbox, float x, float y, float z = 0.0f) const;

    /**
     * @brief Check if bounding box intersects range
     */
    bool intersects_range(const BoundingBox& bbox,
                          float min_x, float min_y, float min_z,
                          float max_x, float max_y, float max_z) const;

    /**
     * @brief Check if bounding box intersects sphere
     */
    bool intersects_sphere(const BoundingBox& bbox,
                           float center_x, float center_y, float center_z,
                           float radius) const;

    /**
     * @brief Compute center of bounding box
     */
    std::array<float, 3> compute_center(const BoundingBox& bbox) const;

    /**
     * @brief Compute longest axis of bounding box
     */
    size_t compute_longest_axis(const BoundingBox& bbox) const;

    /**
     * @brief Split simplices by median along axis
     */
    std::pair<std::vector<SimplexID>, std::vector<SimplexID>>
    split_by_median(const std::vector<SimplexID>& simplices, size_t axis);

    /**
     * @brief Split simplices by midpoint along axis
     */
    std::pair<std::vector<SimplexID>, std::vector<SimplexID>>
    split_by_midpoint(const std::vector<SimplexID>& simplices, size_t axis,
                     float midpoint);

    /**
     * @brief Split simplices using SAH
     */
    std::pair<std::vector<SimplexID>, std::vector<SimplexID>>
    split_by_sah(const std::vector<SimplexID>& simplices);

    /**
     * @brief Recursively build BVH tree
     */
    std::unique_ptr<BVHNode> build_tree(const std::vector<SimplexID>& simplices,
                                         size_t depth);

    /**
     * @brief Range query helper
     */
    void range_query_helper(const BVHNode* node,
                            float min_x, float min_y, float min_z,
                            float max_x, float max_y, float max_z,
                            std::vector<SimplexID>& results,
                            size_t& nodes_visited) const;

    /**
     * @brief Point query helper
     */
    void point_query_helper(const BVHNode* node,
                           float x, float y, float z,
                           std::vector<SimplexID>& results,
                           size_t& nodes_visited) const;

    /**
     * @brief Nearest neighbor query helper
     */
    void nearest_neighbor_helper(const BVHNode* node,
                                float x, float y, float z,
                                NearestResult& result) const;

    /**
     * @brief Distance from point to bounding box
     */
    float distance_to_bbox(const BoundingBox& bbox,
                          float x, float y, float z) const;

    /**
     * @brief Distance from point to simplex
     */
    float distance_to_simplex(SimplexID sid,
                              float x, float y, float z) const;

    /**
     * @ray intersection helper
     */
    void ray_query_helper(const BVHNode* node,
                         const std::array<float, 3>& origin,
                         const std::array<float, 3>& direction,
                         float t_max,
                         std::vector<SimplexID>& results,
                         size_t& nodes_visited) const;

    /**
     * @brief Check if ray intersects bounding box
     */
    bool ray_intersects_bbox(const BoundingBox& bbox,
                            const std::array<float, 3>& origin,
                            const std::array<float, 3>& direction,
                            float t_max) const;

public:
    /**
     * @brief Construct BVH from simplicial complex
     *
     * @param complex Reference to simplicial complex
     * @param strategy Build strategy (default: MEDIAN_SPLIT)
     * @param max_simplices_per_leaf Maximum simplices per leaf node (default: 4)
     * @param max_depth Maximum tree depth (default: 32)
     */
    BVH(const SimplicialComplex& complex,
        BuildStrategy strategy = BuildStrategy::MEDIAN_SPLIT,
        size_t max_simplices_per_leaf = 4,
        size_t max_depth = 32);

    /**
     * @brief Rebuild BVH (useful after complex modifications)
     */
    void rebuild();

    /**
     * @brief Incremental update of BVH
     *
     * @param added List of added simplices
     * @param removed List of removed simplices
     * @param modified List of modified simplices
     */
    void incremental_update(const std::vector<SimplexID>& added,
                            const std::vector<SimplexID>& removed,
                            const std::vector<SimplexID>& modified = {});

    /**
     * @brief Range query
     *
     * @param min_x, min_y, min_z Minimum coordinates
     * @param max_x, max_y, max_z Maximum coordinates
     * @return Query result with found simplices and timing
     */
    QueryResult range_query(float min_x, float min_y, float min_z,
                            float max_x, float max_y, float max_z);

    /**
     * @brief Point query (find all simplices containing point)
     *
     * @param x, y, z Point coordinates
     * @return Query result with found simplices and timing
     */
    QueryResult point_query(float x, float y, float z);

    /**
     * @brief Sphere query (find all simplices intersecting sphere)
     *
     * @param center_x, center_y, center_z Sphere center
     * @param radius Sphere radius
     * @return Query result with found simplices and timing
     */
    QueryResult sphere_query(float center_x, float center_y, float center_z,
                              float radius);

    /**
     * @brief Nearest neighbor query
     *
     * @param x, y, z Query point
     * @return Nearest neighbor result
     */
    NearestResult nearest_neighbor(float x, float y, float z);

    /**
     * @brief K-nearest neighbors query
     *
     * @param x, y, z Query point
     * @param k Number of neighbors to return
     * @return Vector of nearest neighbors sorted by distance
     */
    std::vector<std::pair<SimplexID, double>>
    k_nearest_neighbors(float x, float y, float z, size_t k);

    /**
     * @brief Ray query (find all simplices intersecting ray)
     *
     * @param origin Ray origin
     * @param direction Ray direction
     * @param t_max Maximum ray length (default: infinity)
     * @return Query result with found simplices and timing
     */
    QueryResult ray_query(const std::array<float, 3>& origin,
                          const std::array<float, 3>& direction,
                          float t_max = std::numeric_limits<float>::max());

    /**
     * @brief Collision detection with another BVH
     *
     * @param other Other BVH
     * @return Vector of colliding simplex pairs
     */
    std::vector<std::pair<SimplexID, SimplexID>>
    find_collisions(const BVH& other) const;

    /**
     * @brief Get tree statistics
     *
     * @return Map containing various statistics
     */
    std::unordered_map<std::string, size_t> get_statistics() const;

    /**
     * @brief Get maximum depth of tree
     */
    size_t get_max_depth() const;

    /**
     * @brief Get total number of nodes
     */
    size_t get_node_count() const;

    /**
     * @brief Get total number of leaf nodes
     */
    size_t get_leaf_count() const;

    /**
     * @brief Check if BVH is valid
     */
    bool is_valid() const;

    /**
     * @brief Optimize BVH by rebalancing
     */
    void optimize();

    /**
     * @brief Clear BVH
     */
    void clear();
};

} // namespace cebu

#endif // CEBU_BVH_H
