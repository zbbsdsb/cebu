#include "cebu/refinement.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace cebu {

// ============================================================================
// Implementation of SimplicialComplexRefinement
// ============================================================================

template<typename LabelType>
RefinementResult SimplicialComplexRefinement<LabelType>::refine_edge(
    EdgeID edge_id,
    const RefinementOptions<LabelType>& options) {
    
    RefinementResult result;
    
    // Check if edge exists and is 1-dimensional
    if (!this->has_simplex(edge_id) || this->dimension(edge_id) != 1) {
        return result;
    }
    
    // Get the edge's vertices
    auto vertices = this->get_vertices_of_simplex(edge_id);
    if (vertices.size() != 2) {
        return result; // Not a valid edge
    }
    
    VertexID v0 = static_cast<VertexID>(vertices[0]);
    VertexID v1 = static_cast<VertexID>(vertices[1]);
    
    // Create midpoint vertex
    VertexID mid = this->add_vertex();
    result.new_vertices_count++;
    
    // Store parent edge relationship
    result.new_vertex_parent_edge[mid] = edge_id;
    
    // Get original label
    LabelType original_label;
    bool has_label = this->has_label(edge_id);
    if (has_label) {
        original_label = this->get_label(edge_id);
    }
    
    // Remove original edge
    this->remove_simplex(edge_id, false);
    
    // Create two new edges: v0-mid and mid-v1
    EdgeID edge0 = this->add_edge(v0, mid);
    EdgeID edge1 = this->add_edge(mid, v1);
    
    result.new_simplices_count = 2;
    result.original_to_children[edge_id] = {edge0, edge1};
    
    // Apply label inheritance
    if (has_label) {
        if (options.label_strategy == LabelInheritanceStrategy::INHERIT_COPY) {
            this->set_label(edge0, original_label);
            this->set_label(edge1, original_label);
        } else if (options.label_strategy == LabelInheritanceStrategy::INHERIT_INTERPOLATE) {
            LabelType label0 = interpolate_label(original_label, original_label, 0.5);
            LabelType label1 = interpolate_label(original_label, original_label, 0.5);
            this->set_label(edge0, label0);
            this->set_label(edge1, label1);
        } else if (options.label_strategy == LabelInheritanceStrategy::INHERIT_CUSTOM && 
                   options.custom_label_func) {
            this->set_label(edge0, options.custom_label_func(original_label, 0, 2));
            this->set_label(edge1, options.custom_label_func(original_label, 1, 2));
        }
    }
    
    // Update refinement levels
    update_children_levels({edge0, edge1}, options);
    
    // Handle containing simplices (faces, etc.)
    // Find all simplices containing the original edge
    // This is complex - for now we handle the simple case
    
    return result;
}

template<typename LabelType>
RefinementResult SimplicialComplexRefinement<LabelType>::refine_triangle(
    SimplexID triangle_id,
    const RefinementOptions<LabelType>& options) {
    
    RefinementResult result;
    
    // Check if triangle exists and is 2-dimensional
    if (!this->has_simplex(triangle_id) || this->dimension(triangle_id) != 2) {
        return result;
    }
    
    // Get the triangle's vertices
    auto vertices = this->get_vertices_of_simplex(triangle_id);
    if (vertices.size() != 3) {
        return result; // Not a valid triangle
    }
    
    VertexID v0 = static_cast<VertexID>(vertices[0]);
    VertexID v1 = static_cast<VertexID>(vertices[1]);
    VertexID v2 = static_cast<VertexID>(vertices[2]);
    
    // Create midpoint vertices on each edge
    VertexID mid01 = this->add_vertex(); // Between v0 and v1
    VertexID mid12 = this->add_vertex(); // Between v1 and v2
    VertexID mid20 = this->add_vertex(); // Between v2 and v0
    
    result.new_vertices_count = 3;
    
    // Get original label
    LabelType original_label;
    bool has_label = this->has_label(triangle_id);
    if (has_label) {
        original_label = this->get_label(triangle_id);
    }
    
    // Remove original triangle
    this->remove_simplex(triangle_id, false);
    
    // Create new edges: connecting midpoints to each other and vertices
    // We create a total of 12 edges (9 boundary + 3 interior)
    // Actually for this refinement we need specific edges:
    // v0-mid01, v1-mid01, v1-mid12, v2-mid12, v2-mid20, v0-mid20
    // mid01-mid12, mid12-mid20, mid20-mid01
    
    this->add_edge(v0, mid01);
    this->add_edge(v1, mid01);
    this->add_edge(v1, mid12);
    this->add_edge(v2, mid12);
    this->add_edge(v2, mid20);
    this->add_edge(v0, mid20);
    
    EdgeID e01_12 = this->add_edge(mid01, mid12);
    EdgeID e12_20 = this->add_edge(mid12, mid20);
    EdgeID e20_01 = this->add_edge(mid20, mid01);
    
    // Create 4 new triangles
    // Triangle 1: v0, mid01, mid20
    SimplexID tri0 = this->add_triangle(v0, mid01, mid20);
    
    // Triangle 2: v1, mid12, mid01
    SimplexID tri1 = this->add_triangle(v1, mid12, mid01);
    
    // Triangle 3: v2, mid20, mid12
    SimplexID tri2 = this->add_triangle(v2, mid20, mid12);
    
    // Triangle 4: mid01, mid12, mid20 (center)
    SimplexID tri3 = this->add_triangle(mid01, mid12, mid20);
    
    result.new_simplices_count = 4;
    result.original_to_children[triangle_id] = {tri0, tri1, tri2, tri3};
    
    // Apply label inheritance
    if (has_label) {
        std::vector<SimplexID> children = {tri0, tri1, tri2, tri3};
        apply_label_inheritance(triangle_id, children, options);
    }
    
    // Update refinement levels
    update_children_levels({tri0, tri1, tri2, tri3}, options);
    
    return result;
}

template<typename LabelType>
RefinementResult SimplicialComplexRefinement<LabelType>::refine_simplex(
    SimplexID simplex_id,
    const RefinementOptions<LabelType>& options) {
    
    // Check max level
    if (options.current_level >= options.max_level) {
        return {};
    }
    
    int dim = this->dimension(simplex_id);
    
    switch (dim) {
        case 1:
            return refine_edge(static_cast<EdgeID>(simplex_id), options);
        case 2:
            return refine_triangle(simplex_id, options);
        default:
            // For higher dimensions, we could implement more general refinement
            return {};
    }
}

template<typename LabelType>
RefinementResult SimplicialComplexRefinement<LabelType>::refine_region(
    const std::vector<SimplexID>& simplex_ids,
    const RefinementOptions<LabelType>& options) {
    
    RefinementResult combined;
    
    for (SimplexID id : simplex_ids) {
        RefinementResult sub_result = refine_simplex(id, options);
        
        // Merge results
        combined.original_to_children.insert(
            sub_result.original_to_children.begin(),
            sub_result.original_to_children.end());
        combined.new_vertex_parent_edge.insert(
            sub_result.new_vertex_parent_edge.begin(),
            sub_result.new_vertex_parent_edge.end());
        combined.new_simplices_count += sub_result.new_simplices_count;
        combined.new_vertices_count += sub_result.new_vertices_count;
    }
    
    return combined;
}

template<typename LabelType>
bool SimplicialComplexRefinement<LabelType>::coarsen_edge(
    VertexID middle_vertex_id,
    const RefinementOptions<LabelType>& options) {
    
    // Check if vertex exists and is a midpoint (degree 2)
    if (!is_refinement_midpoint(middle_vertex_id)) {
        return false;
    }
    
    // Get the two edges containing this vertex
    auto containing_edges = this->get_simplices_containing_vertex(middle_vertex_id);
    std::vector<EdgeID> edges;
    for (SimplexID sid : containing_edges) {
        if (this->dimension(sid) == 1) {
            edges.push_back(static_cast<EdgeID>(sid));
        }
    }
    
    if (edges.size() != 2) {
        return false;
    }
    
    EdgeID e0 = edges[0];
    EdgeID e1 = edges[1];
    
    // Get vertices of both edges
    auto v0 = this->get_vertices_of_simplex(e0);
    auto v1 = this->get_vertices_of_simplex(e1);
    
    // Find the endpoints (not the middle vertex)
    VertexID endpoint0 = static_cast<VertexID>(v0[0] == middle_vertex_id ? v0[1] : v0[0]);
    VertexID endpoint1 = static_cast<VertexID>(v1[0] == middle_vertex_id ? v1[1] : v1[0]);
    
    // Remove both edges and middle vertex
    this->remove_simplex(e0, false);
    this->remove_simplex(e1, false);
    this->remove_vertex(middle_vertex_id, false);
    
    // Create new edge connecting endpoints
    this->add_edge(endpoint0, endpoint1);
    
    return true;
}

template<typename LabelType>
bool SimplicialComplexRefinement<LabelType>::coarsen_triangle(
    VertexID center_vertex_id,
    const RefinementOptions<LabelType>& options) {
    
    // Check if vertex exists and has degree 3 (connected to 3 other vertices)
    auto containing_edges = this->get_simplices_containing_vertex(center_vertex_id);
    std::vector<EdgeID> edges;
    for (SimplexID sid : containing_edges) {
        if (this->dimension(sid) == 1) {
            edges.push_back(static_cast<EdgeID>(sid));
        }
    }
    
    if (edges.size() != 3) {
        return false;
    }
    
    // Get the three corner vertices (connected to center)
    std::vector<VertexID> corners;
    for (EdgeID e : edges) {
        auto vertices = this->get_vertices_of_simplex(e);
        VertexID corner = static_cast<VertexID>(
            vertices[0] == center_vertex_id ? vertices[1] : vertices[0]);
        corners.push_back(corner);
    }
    
    // Remove old triangles (should be 4 of them)
    auto containing_faces = this->get_simplices_containing_vertex(center_vertex_id);
    for (SimplexID face_id : containing_faces) {
        if (this->dimension(face_id) == 2) {
            this->remove_simplex(face_id, false);
        }
    }
    
    // Remove edges and center vertex
    for (EdgeID e : edges) {
        this->remove_simplex(e, false);
    }
    this->remove_vertex(center_vertex_id, false);
    
    // Create new triangle connecting the three corners
    this->add_triangle(corners[0], corners[1], corners[2]);
    
    return true;
}

template<typename LabelType>
size_t SimplicialComplexRefinement<LabelType>::coarsen_region(
    std::function<bool(SimplexID)> predicate,
    const RefinementOptions<LabelType>& options) {
    
    size_t count = 0;
    
    // Collect vertices to coarsen (to avoid modification during iteration)
    std::vector<VertexID> vertices_to_coarsen;
    for (const auto& [id, _] : this->get_all_simplices()) {
        if (this->dimension(id) == 0 && predicate(id)) {
            vertices_to_coarsen.push_back(static_cast<VertexID>(id));
        }
    }
    
    for (VertexID vid : vertices_to_coarsen) {
        if (is_refinement_midpoint(vid)) {
            if (coarsen_edge(vid, options)) {
                count++;
            }
        } else if (this->get_simplices_containing_vertex(vid).size() == 3) {
            if (coarsen_triangle(vid, options)) {
                count++;
            }
        }
    }
    
    return count;
}

template<typename LabelType>
bool SimplicialComplexRefinement<LabelType>::is_refinement_midpoint(
    VertexID vertex_id) const {
    
    auto containing = this->get_simplices_containing_vertex(vertex_id);
    
    // Count edges
    size_t edge_count = 0;
    for (SimplexID sid : containing) {
        if (this->dimension(sid) == 1) {
            edge_count++;
        }
    }
    
    return edge_count == 2;
}

template<typename LabelType>
int SimplicialComplexRefinement<LabelType>::get_refinement_level(
    SimplexID simplex_id) const {
    
    auto it = refinement_levels_.find(simplex_id);
    if (it != refinement_levels_.end()) {
        return it->second;
    }
    return 0;
}

template<typename LabelType>
void SimplicialComplexRefinement<LabelType>::set_refinement_level(
    SimplexID simplex_id, int level) {
    
    refinement_levels_[simplex_id] = level;
}

template<typename LabelType>
std::vector<SimplexID> SimplicialComplexRefinement<LabelType>::get_simplices_at_level(
    int level) const {
    
    std::vector<SimplexID> result;
    
    for (const auto& [id, lvl] : refinement_levels_) {
        if (lvl == level) {
            result.push_back(id);
        }
    }
    
    return result;
}

template<typename LabelType>
RefinementResult SimplicialComplexRefinement<LabelType>::adaptive_refine(
    std::function<bool(const LabelType&, SimplexID)> predicate,
    const RefinementOptions<LabelType>& options) {
    
    RefinementResult result;
    
    for (const auto& [id, _] : this->get_all_simplices()) {
        if (this->has_label(id)) {
            LabelType label = this->get_label(id);
            if (predicate(label, id)) {
                auto sub_result = refine_simplex(id, options);
                
                // Merge results
                result.original_to_children.insert(
                    sub_result.original_to_children.begin(),
                    sub_result.original_to_children.end());
                result.new_vertex_parent_edge.insert(
                    sub_result.new_vertex_parent_edge.begin(),
                    sub_result.new_vertex_parent_edge.end());
                result.new_simplices_count += sub_result.new_simplices_count;
                result.new_vertices_count += sub_result.new_vertices_count;
            }
        }
    }
    
    return result;
}

template<typename LabelType>
size_t SimplicialComplexRefinement<LabelType>::adaptive_coarsen(
    std::function<bool(const LabelType&, SimplexID)> predicate,
    const RefinementOptions<LabelType>& options) {
    
    size_t count = 0;
    
    for (const auto& [id, _] : this->get_all_simplices()) {
        if (this->has_label(id)) {
            LabelType label = this->get_label(id);
            if (predicate(label, id)) {
                // Try to coarsen this simplex
                if (this->dimension(id) == 0) {
                    VertexID vid = static_cast<VertexID>(id);
                    if (is_refinement_midpoint(vid)) {
                        if (coarsen_edge(vid, options)) {
                            count++;
                        }
                    }
                }
            }
        }
    }
    
    return count;
}

// ============================================================================
// Private helper methods
// ============================================================================

template<typename LabelType>
void SimplicialComplexRefinement<LabelType>::apply_label_inheritance(
    SimplexID parent_id,
    const std::vector<SimplexID>& child_ids,
    const RefinementOptions<LabelType>& options) {
    
    if (!this->has_label(parent_id)) {
        return;
    }
    
    LabelType parent_label = this->get_label(parent_id);
    
    switch (options.label_strategy) {
        case LabelInheritanceStrategy::INHERIT_COPY:
            for (SimplexID child_id : child_ids) {
                this->set_label(child_id, parent_label);
            }
            break;
            
        case LabelInheritanceStrategy::INHERIT_INTERPOLATE:
            // Distribute label evenly
            for (SimplexID child_id : child_ids) {
                this->set_label(child_id, parent_label);
            }
            break;
            
        case LabelInheritanceStrategy::INHERIT_DISTRIBUTE:
            // Split label value evenly (for numeric types)
            for (SimplexID child_id : child_ids) {
                LabelType distributed_label = parent_label / static_cast<LabelType>(child_ids.size());
                this->set_label(child_id, distributed_label);
            }
            break;
            
        case LabelInheritanceStrategy::INHERIT_CUSTOM:
            if (options.custom_label_func) {
                for (size_t i = 0; i < child_ids.size(); ++i) {
                    LabelType child_label = options.custom_label_func(parent_label, i, child_ids.size());
                    this->set_label(child_ids[i], child_label);
                }
            }
            break;
    }
    
    // Optionally preserve parent labels
    if (options.preserve_parent_labels) {
        this->set_label(parent_id, parent_label);
    }
}

template<typename LabelType>
LabelType SimplicialComplexRefinement<LabelType>::interpolate_label(
    const LabelType& label1,
    const LabelType& label2,
    double t) const {
    
    // Default implementation assumes LabelType supports arithmetic
    // For custom types, this should be specialized
    return static_cast<LabelType>((1.0 - t) * label1 + t * label2);
}

template<typename LabelType>
void SimplicialComplexRefinement<LabelType>::update_children_levels(
    const std::vector<SimplexID>& child_ids,
    const RefinementOptions<LabelType>& options) {
    
    int new_level = options.current_level + 1;
    for (SimplexID child_id : child_ids) {
        set_refinement_level(child_id, new_level);
    }
}

// ============================================================================
// Implementation of RefinementUtils
// ============================================================================

int RefinementUtils::calculate_ideal_level(const std::vector<VertexID>& vertices) {
    // This is a placeholder - in a real implementation, this would
    // analyze local geometry, curvature, or other metrics
    return 0;
}

double RefinementUtils::estimate_fractal_dimension(
    const std::vector<SimplexID>& simplices) {
    
    // Placeholder implementation
    // In a real implementation, this would use box-counting or similar methods
    if (simplices.empty()) {
        return 0.0;
    }
    
    return 2.0; // Default to 2D for triangles
}

// Explicit template instantiations
template class SimplicialComplexRefinement<double>;
template class SimplicialComplexRefinement<float>;
template class SimplicialComplexRefinement<int>;

} // namespace cebu
