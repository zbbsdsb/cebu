#include "cebu/simplicial_complex.h"
#include <sstream>
#include <algorithm>
#include <stdexcept>

namespace cebu {

SimplicialComplex::SimplicialComplex() : next_simplex_id_(0) {}

VertexID SimplicialComplex::add_vertex() {
    // Create vertex as a 0-simplex with a single vertex
    // The vertex ID is the same as its simplex ID
    SimplexID new_id = next_simplex_id_++;
    VertexID vertex_id = static_cast<VertexID>(new_id);

    // Create 0-simplex (vertex)
    auto result = simplices_.emplace(new_id, Simplex({vertex_id}, new_id));
    Simplex& simplex = result.first->second;

    // Update vertex-to-simplex mapping
    vertex_to_simplices_[vertex_id] = std::unordered_set<SimplexID>({new_id});

    return vertex_id;
}

SimplexID SimplicialComplex::add_edge(VertexID v1, VertexID v2) {
    return add_simplex({v1, v2});
}

SimplexID SimplicialComplex::add_triangle(VertexID v1, VertexID v2, VertexID v3) {
    return add_simplex({v1, v2, v3});
}

SimplexID SimplicialComplex::add_simplex(const std::vector<VertexID>& vertices) {
    // Check vertex count
    if (vertices.empty()) {
        throw std::invalid_argument("Simplex must have at least one vertex");
    }

    // Check if all vertices exist
    for (VertexID v : vertices) {
        if (vertex_to_simplices_.find(v) == vertex_to_simplices_.end()) {
            throw std::invalid_argument("Vertex does not exist");
        }
    }

    // Check if simplex already exists
    std::string hash = compute_hash(vertices);
    for (const auto& [id, simplex] : simplices_) {
        if (simplex.vertices() == vertices) {
            return id;  // Return the ID of the existing simplex
        }
    }

    // Create new simplex
    SimplexID new_id = next_simplex_id_++;
    auto result = simplices_.emplace(new_id, Simplex(vertices, new_id));
    Simplex& simplex = result.first->second;

    // Update vertex-to-simplex mapping
    for (VertexID v : vertices) {
        vertex_to_simplices_[v].insert(new_id);
    }

    // Build adjacency relations: find all simplices adjacent to this simplex
    for (const auto& [id, other] : simplices_) {
        if (id == new_id) continue;

        // Two simplices are adjacent if their intersection is a common face
        // Simplified: if they share at least one vertex, they are adjacent
        const auto& other_vertices = other.vertices();
        bool adjacent = false;

        for (VertexID v1 : vertices) {
            for (VertexID v2 : other_vertices) {
                if (v1 == v2) {
                    adjacent = true;
                    break;
                }
            }
            if (adjacent) break;
        }

        if (adjacent) {
            simplex.add_neighbor(id);
            const_cast<Simplex&>(other).add_neighbor(new_id);
        }
    }

    return new_id;
}

std::vector<SimplexID> SimplicialComplex::get_simplices_of_dimension(size_t dimension) const {
    std::vector<SimplexID> result;
    for (const auto& [id, simplex] : simplices_) {
        if (simplex.dimension() == dimension) {
            result.push_back(id);
        }
    }
    return result;
}

const Simplex& SimplicialComplex::get_simplex(SimplexID id) const {
    auto it = simplices_.find(id);
    if (it == simplices_.end()) {
        throw std::out_of_range("Simplex ID does not exist");
    }
    return it->second;
}

std::vector<SimplexID> SimplicialComplex::get_simplices_containing_vertex(
    VertexID vertex_id) const {
    std::vector<SimplexID> result;
    auto it = vertex_to_simplices_.find(vertex_id);
    if (it != vertex_to_simplices_.end()) {
        result.assign(it->second.begin(), it->second.end());
    }
    return result;
}

std::vector<SimplexID> SimplicialComplex::get_adjacent_simplices(
    SimplexID simplex_id) const {
    const Simplex& simplex = get_simplex(simplex_id);
    return std::vector<SimplexID>(simplex.neighbors().begin(),
                                   simplex.neighbors().end());
}

std::vector<SimplexID> SimplicialComplex::get_facets(SimplexID simplex_id) const {
    const Simplex& simplex = get_simplex(simplex_id);
    const auto& vertices = simplex.vertices();

    std::vector<SimplexID> result;

    // For a k-simplex, its faces are all (k-1)-simplices
    // That is, all combinations after removing one vertex
    if (simplex.dimension() == 0) {
        // 0-simplex (vertex) has no faces
        return result;
    }

    for (size_t i = 0; i < vertices.size(); ++i) {
        // Create a face: remove the i-th vertex
        std::vector<VertexID> facet_vertices;
        for (size_t j = 0; j < vertices.size(); ++j) {
            if (j != i) {
                facet_vertices.push_back(vertices[j]);
            }
        }

        // Check if this face already exists
        std::string facet_hash = compute_hash(facet_vertices);
        for (const auto& [id, s] : simplices_) {
            if (s.vertices() == facet_vertices) {
                result.push_back(id);
                break;
            }
        }
    }

    return result;
}

std::string SimplicialComplex::compute_hash(
    const std::vector<VertexID>& vertices) const {
    std::ostringstream oss;
    for (VertexID v : vertices) {
        oss << v << ",";
    }
    return oss.str();
}

void SimplicialComplex::remove_from_adjacency(SimplexID simplex_id) {
    // Remove this simplex from all its neighbors' adjacency lists
    auto it = simplices_.find(simplex_id);
    if (it == simplices_.end()) return;

    const Simplex& simplex = it->second;
    for (SimplexID neighbor_id : simplex.neighbors()) {
        auto neighbor_it = simplices_.find(neighbor_id);
        if (neighbor_it != simplices_.end()) {
            const_cast<Simplex&>(neighbor_it->second).remove_neighbor(simplex_id);
        }
    }
}

void SimplicialComplex::remove_from_vertex_mappings(SimplexID simplex_id) {
    // Remove this simplex from all vertices that contain it
    auto it = simplices_.find(simplex_id);
    if (it == simplices_.end()) return;

    const Simplex& simplex = it->second;
    for (VertexID v : simplex.vertices()) {
        auto v_it = vertex_to_simplices_.find(v);
        if (v_it != vertex_to_simplices_.end()) {
            v_it->second.erase(simplex_id);
            if (v_it->second.empty()) {
                vertex_to_simplices_.erase(v_it);
            }
        }
    }
}

bool SimplicialComplex::remove_simplex(SimplexID simplex_id, bool cascade) {
    auto it = simplices_.find(simplex_id);
    if (it == simplices_.end()) {
        return false;
    }

    if (cascade) {
        // Collect all simplices that contain this simplex as a face
        std::vector<SimplexID> to_remove;
        for (const auto& [id, simplex] : simplices_) {
            if (id == simplex_id) continue;

            const auto& vertices = simplex.vertices();
            const auto& target_vertices = it->second.vertices();

            // Check if the target simplex is a face of this simplex
            bool contains = true;
            if (vertices.size() > target_vertices.size()) {
                for (VertexID v : target_vertices) {
                    if (std::find(vertices.begin(), vertices.end(), v) == vertices.end()) {
                        contains = false;
                        break;
                    }
                }
                if (contains) {
                    to_remove.push_back(id);
                }
            }
        }

        // Recursively remove simplices that contain this simplex
        for (SimplexID id : to_remove) {
            remove_simplex(id, true);
        }
    }

    // Remove from adjacency relations and vertex mappings
    remove_from_adjacency(simplex_id);
    remove_from_vertex_mappings(simplex_id);

    // Remove the simplex itself
    simplices_.erase(it);

    return true;
}

bool SimplicialComplex::remove_vertex(VertexID vertex_id, bool cascade) {
    // Check if vertex exists
    if (vertex_to_simplices_.find(vertex_id) == vertex_to_simplices_.end()) {
        return false;
    }

    if (cascade) {
        // Remove all simplices that contain this vertex
        auto it = vertex_to_simplices_.find(vertex_id);
        if (it != vertex_to_simplices_.end()) {
            std::vector<SimplexID> to_remove(it->second.begin(), it->second.end());
            for (SimplexID simplex_id : to_remove) {
                remove_simplex(simplex_id, false);
            }
        }
    }

    // Remove vertex simplex
    remove_simplex(vertex_id, false);

    // Remove vertex mapping
    vertex_to_simplices_.erase(vertex_id);

    return true;
}

std::vector<VertexID> SimplicialComplex::get_vertices() const {
    std::vector<VertexID> vertices;
    vertices.reserve(vertex_to_simplices_.size());
    for (const auto& [vertex_id, simplices] : vertex_to_simplices_) {
        vertices.push_back(vertex_id);
    }
    return vertices;
}

size_t SimplicialComplex::vertex_count() const {
    return vertex_to_simplices_.size();
}

} // namespace cebu
