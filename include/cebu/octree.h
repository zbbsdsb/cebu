#ifndef CEBU_OCTREE_H
#define CEBU_OCTREE_H

#include "simplicial_complex.h"
#include "simplicial_complex_labeled.h"
#include <vector>
#include <array>
#include <memory>
#include <functional>
#include <stack>

namespace cebu {

/**
 * @brief Octree node
 */
struct OctreeNode {
    std::array<double, 3> center;    // Center of octant
    double extent;                    // Half-size of octant
    std::vector<SimplexID> simplices; // Simplices in this node
    std::array<std::unique_ptr<OctreeNode>, 8> children; // 8 children
    bool is_leaf;

    OctreeNode(const std::array<double, 3>& c, double e)
        : center(c), extent(e), is_leaf(true) {}

    /**
     * @brief Get bounding box of this node
     */
    void get_bounds(std::array<double, 3>& min, std::array<double, 3>& max) const {
        for (int i = 0; i < 3; ++i) {
            min[i] = center[i] - extent;
            max[i] = center[i] + extent;
        }
    }

    /**
     * @brief Check if point is inside this node
     */
    bool contains(const std::array<double, 3>& point) const {
        for (int i = 0; i < 3; ++i) {
            if (point[i] < center[i] - extent || point[i] > center[i] + extent) {
                return false;
            }
        }
        return true;
    }
};

/**
 * @brief Octree query result
 */
struct OctreeQueryResult {
    std::vector<SimplexID> simplex_ids;
    size_t nodes_visited;
    size_t depth;

    OctreeQueryResult() : nodes_visited(0), depth(0) {}
};

/**
 * @brief High-performance Octree spatial index
 * 
 * Provides uniform space subdivision for efficient spatial queries.
 * Supports LOD (Level of Detail) and adaptive refinement.
 */
class Octree {
public:
    Octree();
    explicit Octree(const SimplicialComplex& complex);
    explicit Octree(const SimplicialComplexLabeled<>& complex);
    
    ~Octree();

    // Build octree from simplicial complex
    void build(const SimplicialComplex& complex);
    void build(const SimplicialComplexLabeled<>& complex);
    
    // Rebuild octree
    void rebuild();
    
    // Point queries
    bool contains(const std::array<double, 3>& point) const;
    std::vector<SimplexID> query_point(const std::array<double, 3>& point) const;
    
    // Range queries
    OctreeQueryResult query_range(const std::array<double, 3>& min,
                                 const std::array<double, 3>& max) const;
    OctreeQueryResult query_sphere(const std::array<double, 3>& center, double radius) const;
    
    // Nearest neighbor
    std::vector<SimplexID> nearest_neighbors(const std::array<double, 3>& point,
                                             size_t k) const;
    
    // Frustum culling
    std::vector<SimplexID> query_frustum(const std::array<double, 16>& view_proj) const;

    // Update operations
    void insert_simplex(SimplexID id);
    void remove_simplex(SimplexID id);
    void update_simplex(SimplexID id);
    
    // LOD operations
    void set_lod_level(int level);
    int get_lod_level() const;
    std::vector<SimplexID> get_lod_simplices(int lod_level) const;

    // Statistics
    size_t get_node_count() const;
    size_t get_depth() const;
    size_t get_leaf_count() const;
    size_t get_simplex_count() const;
    size_t get_max_simplices_per_node() const;
    OctreeQueryResult get_statistics() const;

    // Parameters
    void set_max_simplices_per_node(size_t max);
    size_t get_max_simplices_per_node_param() const;
    void set_max_depth(size_t depth);
    size_t get_max_depth_param() const;

    // Validation
    bool validate() const;
    void print_tree() const;

private:
    std::unique_ptr<OctreeNode> root_;
    const SimplicialComplex* complex_;
    const SimplicialComplexLabeled<>* labeled_complex_;
    size_t max_simplices_per_node_;
    size_t max_depth_;
    int current_lod_level_;

    // Build methods
    void build_recursive(OctreeNode* node, const std::vector<SimplexID>& simplex_ids,
                        size_t depth);
    void subdivide(OctreeNode* node);
    int get_child_index(const OctreeNode* node, const std::array<double, 3>& point) const;

    // Query methods
    void query_point_recursive(const OctreeNode* node, const std::array<double, 3>& point,
                               std::vector<SimplexID>& results, size_t& depth) const;
    void query_range_recursive(const OctreeNode* node, const std::array<double, 3>& min,
                               const std::array<double, 3>& max,
                               OctreeQueryResult& results) const;
    void query_sphere_recursive(const OctreeNode* node, const std::array<double, 3>& center,
                                double radius_sq, OctreeQueryResult& results) const;

    // Utility methods
    std::array<double, 3> get_simplex_position(SimplexID id) const;
    void compute_bounds(const SimplicialComplex& complex,
                       std::array<double, 3>& min, std::array<double, 3>& max) const;
    void compute_bounds(const SimplicialComplexLabeled<>& complex,
                       std::array<double, 3>& min, std::array<double, 3>& max) const;
    size_t count_nodes(const OctreeNode* node) const;
    size_t get_max_depth(const OctreeNode* node, size_t current) const;
    size_t count_leaves(const OctreeNode* node) const;
    size_t count_simplices(const OctreeNode* node) const;
    bool validate_recursive(const OctreeNode* node) const;
    void print_tree_recursive(const OctreeNode* node, int depth) const;
};

} // namespace cebu

#endif // CEBU_OCTREE_H
