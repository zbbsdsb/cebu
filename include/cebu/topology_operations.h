#ifndef CEBU_TOPOLOGY_OPERATIONS_H
#define CEBU_TOPOLOGY_OPERATIONS_H

#include "cebu/simplicial_complex.h"
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <stdexcept>

namespace cebu {

/// Provides topological operations on simplicial complexes
class TopologyOperations {
public:
    /// Glue two vertices together (identify them as the same vertex)
    ///
    /// This operation merges v1 into v2:
    /// - All simplices containing v1 are updated to contain v2 instead
    /// - Vertex v1 is removed from the complex
    /// - v1 and v2 become indistinguishable in the complex
    ///
    /// @param complex The simplicial complex to modify
    /// @param v1 Vertex ID to merge (will be removed)
    /// @param v2 Vertex ID to merge into (will be kept)
    /// @returns ID of the resulting vertex (v2)
    /// @throws std::invalid_argument if v1 == v2 or vertices don't exist
    static VertexID glue_vertices(SimplicialComplex& complex, VertexID v1, VertexID v2) {
        if (v1 == v2) {
            throw std::invalid_argument("Cannot glue vertex to itself");
        }

        // Check if vertices exist by checking if they are in vertex_to_simplices mapping
        // We use has_simplex since vertices are 0-simplices
        if (!complex.has_simplex(v1)) {
            throw std::invalid_argument("Vertex v1 does not exist");
        }
        if (!complex.has_simplex(v2)) {
            throw std::invalid_argument("Vertex v2 does not exist");
        }

        // Get all simplices containing v1 (excluding v1 itself as a 0-simplex)
        auto simplices_with_v1 = complex.get_simplices_containing_vertex(v1);

        // Remove v1 itself (the 0-simplex) from the list
        auto it = std::find(simplices_with_v1.begin(), simplices_with_v1.end(), v1);
        if (it != simplices_with_v1.end()) {
            simplices_with_v1.erase(it);
        }

        // For each simplex containing v1, replace v1 with v2
        // We need to rebuild these simplices
        for (SimplexID sid : simplices_with_v1) {
            const Simplex& simplex = complex.get_simplex(sid);
            std::vector<VertexID> new_vertices;

            for (VertexID vid : simplex.vertices()) {
                if (vid == v1) {
                    new_vertices.push_back(v2);
                } else {
                    new_vertices.push_back(vid);
                }
            }

            // Remove the old simplex and add the updated one
            complex.remove_simplex(sid, false);  // Don't cascade
            complex.add_simplex(new_vertices);
        }

        // Remove vertex v1 (the 0-simplex)
        complex.remove_vertex(v1, false);

        return v2;
    }

    /// Compute the boundary of the complex
    ///
    /// The boundary consists of all simplices that are not faces of any
    /// higher-dimensional simplex.
    ///
    /// @param complex The simplicial complex
    /// @returns Vector of simplex IDs that form the boundary
    static std::vector<SimplexID> compute_boundary(const SimplicialComplex& complex) {
        std::vector<SimplexID> boundary;

        for (const auto& pair : complex.get_simplices()) {
            SimplexID sid = pair.first;
            const Simplex& simplex = pair.second;

            // Check if this simplex is a face of any higher-dimensional simplex
            bool is_face = false;
            for (const auto& other_pair : complex.get_simplices()) {
                SimplexID other_sid = other_pair.first;
                const Simplex& other = other_pair.second;

                // Skip if other is the same simplex or lower dimension
                if (other_sid == sid) continue;
                if (other.dimension() <= simplex.dimension()) continue;

                // Check if this simplex is a face of the other simplex
                if (is_face_of(simplex, other)) {
                    is_face = true;
                    break;
                }
            }

            // If not a face of any higher-dimensional simplex, it's on the boundary
            if (!is_face) {
                boundary.push_back(sid);
            }
        }

        return boundary;
    }

    /// Get all simplices of a specific dimension
    ///
    /// @param complex The simplicial complex
    /// @param dimension The dimension to filter by
    /// @returns Vector of simplex IDs with the given dimension
    static std::vector<SimplexID> get_simplices_of_dimension(
        const SimplicialComplex& complex, size_t dimension) {

        std::vector<SimplexID> result;
        for (const auto& pair : complex.get_simplices()) {
            if (pair.second.dimension() == dimension) {
                result.push_back(pair.first);
            }
        }
        return result;
    }

    /// Compute the connected components of the complex
    ///
    /// @param complex The simplicial complex
    /// @returns Vector of vectors, where each inner vector is a connected component
    static std::vector<std::vector<SimplexID>> compute_connected_components(
        const SimplicialComplex& complex) {

        std::unordered_set<SimplexID> visited;
        std::vector<std::vector<SimplexID>> components;

        for (const auto& pair : complex.get_simplices()) {
            SimplexID start = pair.first;

            if (visited.count(start)) {
                continue;
            }

            // BFS to find connected component
            std::vector<SimplexID> component;
            std::vector<SimplexID> queue = {start};

            while (!queue.empty()) {
                SimplexID current = queue.back();
                queue.pop_back();

                if (visited.count(current)) {
                    continue;
                }

                visited.insert(current);
                component.push_back(current);

                // Add neighbors to queue
                const Simplex& simplex = complex.get_simplex(current);
                for (SimplexID neighbor : simplex.neighbors()) {
                    if (!visited.count(neighbor)) {
                        queue.push_back(neighbor);
                    }
                }
            }

            if (!component.empty()) {
                components.push_back(component);
            }
        }

        return components;
    }

private:
    /// Check if simplex a is a face of simplex b
    static bool is_face_of(const Simplex& a, const Simplex& b) {
        // a must have fewer vertices than b
        if (a.vertices().size() >= b.vertices().size()) {
            return false;
        }

        // All vertices of a must be in b
        std::unordered_set<VertexID> b_vertices(
            b.vertices().begin(),
            b.vertices().end()
        );

        for (VertexID vid : a.vertices()) {
            if (b_vertices.find(vid) == b_vertices.end()) {
                return false;
            }
        }

        return true;
    }
};

} // namespace cebu

#endif // CEBU_TOPOLOGY_OPERATIONS_H
