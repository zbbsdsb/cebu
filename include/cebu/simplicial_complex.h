#pragma once

#include "cebu/simplex.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <string>

namespace cebu {

/**
 * @brief Dynamic simplicial complex class
 *
 * A simplicial complex is a set of simplices that satisfies:
 * 1. If simplex A is in the complex, all faces of A are also in the complex
 * 2. The intersection of two simplices is a common face (or empty)
 *
 * This implementation provides a dynamic complex that supports both adding and removing simplices.
 */
class SimplicialComplex {
public:
    /**
     * @brief Constructor
     */
    SimplicialComplex();

    /**
     * @brief Add a vertex (0-simplex)
     * @return The ID of the new vertex
     */
    VertexID add_vertex();

    /**
     * @brief Add an edge (1-simplex)
     * @param v1 First vertex ID
     * @param v2 Second vertex ID
     * @return The simplex ID of the new edge
     * @throw std::invalid_argument If vertices do not exist
     */
    SimplexID add_edge(VertexID v1, VertexID v2);

    /**
     * @brief Add a triangle (2-simplex)
     * @param v1 First vertex ID
     * @param v2 Second vertex ID
     * @param v3 Third vertex ID
     * @return The simplex ID of the new triangle
     * @throw std::invalid_argument If vertices do not exist
     */
    SimplexID add_triangle(VertexID v1, VertexID v2, VertexID v3);

    /**
     * @brief Add a k-simplex
     * @param vertices List of vertices that make up the simplex
     * @return The ID of the new simplex
     * @throw std::invalid_argument If vertices do not exist or insufficient count
     */
    SimplexID add_simplex(const std::vector<VertexID>& vertices);

    /**
     * @brief Remove a simplex from the complex
     * @param simplex_id ID of the simplex to remove
     * @param cascade If true, also remove all simplices that contain this simplex
     * @return true if the simplex was removed, false if it didn't exist
     */
    bool remove_simplex(SimplexID simplex_id, bool cascade = false);

    /**
     * @brief Remove a vertex from the complex
     * @param vertex_id ID of the vertex to remove
     * @param cascade If true, also remove all simplices that contain this vertex
     * @return true if the vertex was removed, false if it didn't exist
     */
    bool remove_vertex(VertexID vertex_id, bool cascade = true);

    /**
     * @brief Get all vertex IDs
     */
    std::vector<VertexID> get_vertices() const;

    /**
     * @brief Get all simplices
     */
    const std::unordered_map<SimplexID, Simplex>& get_simplices() const {
        return simplices_;
    }

    /**
     * @brief Get simplices of a specific dimension
     * @param dimension The dimension
     */
    std::vector<SimplexID> get_simplices_of_dimension(size_t dimension) const;

    /**
     * @brief Check if a simplex exists
     * @param id Simplex ID
     */
    bool has_simplex(SimplexID id) const {
        return simplices_.find(id) != simplices_.end();
    }

    /**
     * @brief Get the simplex with the specified ID
     * @param id Simplex ID
     * @return Reference to the simplex
     * @throw std::out_of_range If the simplex does not exist
     */
    const Simplex& get_simplex(SimplexID id) const;

    /**
     * @brief Get all simplices that contain the specified vertex
     * @param vertex_id Vertex ID
     */
    std::vector<SimplexID> get_simplices_containing_vertex(VertexID vertex_id) const;

    /**
     * @brief Get all simplices adjacent to the specified simplex
     * @param simplex_id Simplex ID
     */
    std::vector<SimplexID> get_adjacent_simplices(SimplexID simplex_id) const;

    /**
     * @brief Get all faces of a simplex (boundary simplices of dimension-1)
     * @param simplex_id Simplex ID
     */
    std::vector<SimplexID> get_facets(SimplexID simplex_id) const;

    /**
     * @brief Get the number of simplices
     */
    size_t simplex_count() const { return simplices_.size(); }

    /**
     * @brief Get the number of vertices
     */
    size_t vertex_count() const;

private:
    std::unordered_map<SimplexID, Simplex> simplices_;
    SimplexID next_simplex_id_;

    // Mapping from vertex to simplices that contain it
    std::unordered_map<VertexID, std::unordered_set<SimplexID>> vertex_to_simplices_;

    /**
     * @brief Compute a unique identifier for a simplex based on its vertex list (for duplicate checking)
     */
    std::string compute_hash(const std::vector<VertexID>& vertices) const;

    /**
     * @brief Remove simplex from adjacency relations
     * @param simplex_id ID of the simplex to remove from adjacency lists
     */
    void remove_from_adjacency(SimplexID simplex_id);

    /**
     * @brief Remove simplex from vertex mappings
     * @param simplex_id ID of the simplex to remove
     */
    void remove_from_vertex_mappings(SimplexID simplex_id);
};

} // namespace cebu
