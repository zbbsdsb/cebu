#include "cebu/simplicial_complex_non_hausdorff.h"
#include <stdexcept>

namespace cebu {

// ============================================================================
// SimplicialComplexNonHausdorff Implementation
// ============================================================================

SimplicialComplexNonHausdorff::SimplicialComplexNonHausdorff()
    : SimplicialComplex() {
}

SimplexID SimplicialComplexNonHausdorff::glue(SimplexID simplex_a, SimplexID simplex_b) {
    if (!has_simplex(simplex_a)) {
        throw std::invalid_argument("Simplex A does not exist");
    }
    if (!has_simplex(simplex_b)) {
        throw std::invalid_argument("Simplex B does not exist");
    }

    return equiv_manager_.glue(simplex_a, simplex_b);
}

SimplexID SimplicialComplexNonHausdorff::glue_vertices(VertexID vertex_a, VertexID vertex_b) {
    // Convert vertices to SimplexID (vertices are 0-simplices)
    SimplexID simplex_a = static_cast<SimplexID>(vertex_a);
    SimplexID simplex_b = static_cast<SimplexID>(vertex_b);

    return glue(simplex_a, simplex_b);
}

bool SimplicialComplexNonHausdorff::separate(SimplexID simplex_id) {
    if (!has_simplex(simplex_id)) {
        throw std::invalid_argument("Simplex does not exist");
    }

    return equiv_manager_.separate(simplex_id);
}

bool SimplicialComplexNonHausdorff::are_glued(SimplexID simplex_a, SimplexID simplex_b) const {
    return equiv_manager_.are_glued(simplex_a, simplex_b);
}

bool SimplicialComplexNonHausdorff::is_glued(SimplexID simplex_id) const {
    return equiv_manager_.is_glued(simplex_id);
}

SimplexID SimplicialComplexNonHausdorff::get_representative(SimplexID simplex_id) const {
    return equiv_manager_.find_representative(simplex_id);
}

std::vector<SimplexID> SimplicialComplexNonHausdorff::get_equivalence_class(
    SimplexID simplex_id) const {

    return equiv_manager_.get_equivalence_class(simplex_id);
}

std::vector<SimplexID> SimplicialComplexNonHausdorff::get_adjacent_with_gluing(
    SimplexID simplex_id) const {

    std::unordered_set<SimplexID> result;

    // Get all members of the equivalence class
    auto members = get_equivalence_class(simplex_id);

    // Collect neighbors from all members
    for (SimplexID member : members) {
        const Simplex& simplex = get_simplex(member);
        for (SimplexID neighbor : simplex.neighbors()) {
            result.insert(neighbor);
        }
    }

    // Remove members of the equivalence class from result
    for (SimplexID member : members) {
        result.erase(member);
    }

    return std::vector<SimplexID>(result.begin(), result.end());
}

std::vector<SimplexID> SimplicialComplexNonHausdorff::get_simplices_containing_vertex_with_gluing(
    VertexID vertex_id) const {

    std::unordered_set<SimplexID> result;

    // Convert vertex to SimplexID
    SimplexID vertex_simplex = static_cast<SimplexID>(vertex_id);

    // Get all vertices glued to this one
    auto glued_vertices = get_equivalence_class(vertex_simplex);

    // Collect simplices containing any glued vertex
    for (SimplexID v : glued_vertices) {
        VertexID vid = static_cast<VertexID>(v);
        auto containing = get_simplices_containing_vertex(vid);
        result.insert(containing.begin(), containing.end());
    }

    return std::vector<SimplexID>(result.begin(), result.end());
}

} // namespace cebu
