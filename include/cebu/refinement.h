#ifndef CEBU_REFINEMENT_H
#define CEBU_REFINEMENT_H

#include "cebu/simplicial_complex.h"
#include "cebu/simplicial_complex_labeled.h"
#include <vector>
#include <unordered_map>
#include <functional>

namespace cebu {

/**
 * @brief Result of a refinement operation
 * 
 * Contains information about new simplices created and relationships
 */
struct RefinementResult {
    // Map from original simplex IDs to their refined children
    std::unordered_map<SimplexID, std::vector<SimplexID>> original_to_children;
    
    // Map from new vertices to their parent edges
    std::unordered_map<VertexID, SimplexID> new_vertex_parent_edge;
    
    // Total number of new simplices created
    size_t new_simplices_count = 0;
    
    // Total number of new vertices created
    size_t new_vertices_count = 0;
};

/**
 * @brief Strategy for label inheritance during refinement
 */
enum class LabelInheritanceStrategy {
    /// All children get the same label as parent
    INHERIT_COPY,
    
    /// Labels are interpolated (numeric types)
    INHERIT_INTERPOLATE,
    
    /// Labels are distributed (split values)
    INHERIT_DISTRIBUTE,
    
    /// Use a custom function
    INHERIT_CUSTOM
};

/**
 * @brief Options for refinement operations
 */
template<typename LabelType = double>
struct RefinementOptions {
    /// Strategy for handling labels
    LabelInheritanceStrategy label_strategy = LabelInheritanceStrategy::INHERIT_COPY;
    
    /// Custom label inheritance function
    std::function<LabelType(const LabelType&, size_t, size_t)> custom_label_func;
    
    /// Maximum refinement level (to prevent infinite recursion)
    int max_level = 10;
    
    /// Current refinement level
    int current_level = 0;
    
    /// Preserve labels on parent simplices after refinement
    bool preserve_parent_labels = false;
};

/**
 * @brief Refinement operations for simplicial complexes
 * 
 * Provides methods to locally refine (subdivide) and coarsen (merge) simplices,
 * enabling dynamic adjustment of fractal dimension.
 */
template<typename LabelType = double>
class SimplicialComplexRefinement : public SimplicialComplexLabeled<LabelType> {
public:
    using Base = SimplicialComplexLabeled<LabelType>;
    
    /**
     * @brief Refine a single edge by inserting a midpoint vertex
     * 
     * Converts: v0 --- v1
     * To:     v0 -- vm -- v1
     * 
     * @param edge_id ID of the edge to refine
     * @param options Refinement options
     * @return Result containing new vertex and updated edge information
     */
    RefinementResult refine_edge(SimplexID edge_id, 
                                const RefinementOptions<LabelType>& options = {});
    
    /**
     * @brief Refine a triangle into 4 smaller triangles
     * 
     * Inserts midpoint vertices on each edge and connects them.
     * Original triangle is split into 4 sub-triangles.
     * 
     * @param triangle_id ID of the triangle to refine
     * @param options Refinement options
     * @return Result containing new vertices and triangles
     */
    RefinementResult refine_triangle(SimplexID triangle_id,
                                     const RefinementOptions<LabelType>& options = {});
    
    /**
     * @brief Refine a simplex of any dimension
     * 
     * Automatically chooses the appropriate refinement strategy based on dimension.
     * 
     * @param simplex_id ID of the simplex to refine
     * @param options Refinement options
     * @return Result of refinement
     */
    RefinementResult refine_simplex(SimplexID simplex_id,
                                    const RefinementOptions<LabelType>& options = {});
    
    /**
     * @brief Refine all simplices in a region
     * 
     * @param simplex_ids List of simplex IDs to refine
     * @param options Refinement options
     * @return Combined result of all refinements
     */
    RefinementResult refine_region(const std::vector<SimplexID>& simplex_ids,
                                   const RefinementOptions<LabelType>& options = {});
    
    /**
     * @brief Coarsen a refined edge (undo refine_edge)
     * 
     * Merges a two-edge chain back into a single edge.
     * Only works if the middle vertex has degree 2.
     * 
     * @param middle_vertex_id The vertex to remove (should have degree 2)
     * @param options Refinement options
     * @return true if coarsening was successful
     */
    bool coarsen_edge(VertexID middle_vertex_id,
                     const RefinementOptions<LabelType>& options = {});
    
    /**
     * @brief Coarsen a refined triangle (undo refine_triangle)
     * 
     * Merges 4 sub-triangles back into the original.
     * Only works if the structure matches a refined triangle.
     * 
     * @param center_vertex_id The central vertex from refinement
     * @param options Refinement options
     * @return true if coarsening was successful
     */
    bool coarsen_triangle(VertexID center_vertex_id,
                        const RefinementOptions<LabelType>& options = {});
    
    /**
     * @brief Coarsen a region based on a predicate
     * 
     * @param predicate Function that returns true for simplices to coarsen
     * @param options Refinement options
     * @return Number of simplices removed
     */
    size_t coarsen_region(std::function<bool(SimplexID)> predicate,
                         const RefinementOptions<LabelType>& options = {});
    
    /**
     * @brief Check if a vertex is a refinement midpoint (degree 2)
     * @param vertex_id Vertex to check
     * @return true if vertex has exactly 2 incident edges
     */
    bool is_refinement_midpoint(VertexID vertex_id) const;
    
    /**
     * @brief Get the refinement level of a simplex
     * 
     * Returns how many times this simplex (or its ancestors) have been refined.
     * 
     * @param simplex_id ID of the simplex
     * @return Refinement level (0 means never refined)
     */
    int get_refinement_level(SimplexID simplex_id) const;
    
    /**
     * @brief Set the refinement level of a simplex
     * @param simplex_id ID of the simplex
     * @param level Refinement level
     */
    void set_refinement_level(SimplexID simplex_id, int level);
    
    /**
     * @brief Get all simplices at a specific refinement level
     * @param level Refinement level
     * @return List of simplex IDs at that level
     */
    std::vector<SimplexID> get_simplices_at_level(int level) const;
    
    /**
     * @brief Adaptive refinement based on labels
     * 
     * Refines simplices where the label satisfies a condition.
     * 
     * @param predicate Function to check if a simplex should be refined
     * @param options Refinement options
     * @return Result of refinement
     */
    RefinementResult adaptive_refine(
        std::function<bool(const LabelType&, SimplexID)> predicate,
        const RefinementOptions<LabelType>& options = {});
    
    /**
     * @brief Adaptive coarsening based on labels
     * 
     * Coarsens simplices where the label satisfies a condition.
     * 
     * @param predicate Function to check if a simplex should be coarsened
     * @param options Refinement options
     * @return Number of simplices coarsened
     */
    size_t adaptive_coarsen(
        std::function<bool(const LabelType&, SimplexID)> predicate,
        const RefinementOptions<LabelType>& options = {});

private:
    /// Store refinement level for each simplex
    std::unordered_map<SimplexID, int> refinement_levels_;
    
    /// Helper: apply label inheritance
    void apply_label_inheritance(
        SimplexID parent_id,
        const std::vector<SimplexID>& child_ids,
        const RefinementOptions<LabelType>& options);
    
    /// Helper: interpolate labels for numeric types
    LabelType interpolate_label(const LabelType& label1, 
                               const LabelType& label2,
                               double t) const;
    
    /// Helper: update refinement levels for children
    void update_children_levels(
        const std::vector<SimplexID>& child_ids,
        const RefinementOptions<LabelType>& options);
};

/**
 * @brief Non-templated base for type erasure if needed
 */
class RefinementUtils {
public:
    /// Calculate ideal refinement level based on local geometry
    static int calculate_ideal_level(const std::vector<VertexID>& vertices);
    
    /// Estimate fractal dimension of a region
    static double estimate_fractal_dimension(const std::vector<SimplexID>& simplices);
};

} // namespace cebu

#endif // CEBU_REFINEMENT_H
