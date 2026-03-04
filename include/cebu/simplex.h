#pragma once

#include <vector>
#include <cstdint>
#include <unordered_set>

namespace cebu {

using SimplexID = uint64_t;
using VertexID = uint32_t;

constexpr VertexID INVALID_VERTEX_ID = static_cast<VertexID>(-1);

/**
 * @brief Simplex class representing a simplex (vertex, edge, triangle, etc.)
 *
 * A simplex is the basic element of combinatorial topology:
 * - 0-simplex: vertex
 * - 1-simplex: edge
 * - 2-simplex: triangle
 * - k-simplex: convex hull of k+1 vertices
 */
class Simplex {
public:
    /**
     * @brief Constructor
     * @param vertices List of vertex IDs that make up this simplex
     * @param id Unique ID of the simplex
     */
    explicit Simplex(const std::vector<VertexID>& vertices, SimplexID id);

    /**
     * @brief Get the ID of the simplex
     */
    SimplexID id() const { return id_; }

    /**
     * @brief Get the dimension of the simplex
     * 0-simplex: vertex (1 vertex)
     * 1-simplex: edge (2 vertices)
     * 2-simplex: triangle (3 vertices)
     */
    size_t dimension() const { return vertices_.size() - 1; }

    /**
     * @brief Get the list of vertices that make up this simplex
     */
    const std::vector<VertexID>& vertices() const { return vertices_; }

    /**
     * @brief Get the set of neighbor simplex IDs
     */
    const std::unordered_set<SimplexID>& neighbors() const { return neighbors_; }

    /**
     * @brief Add a neighboring simplex
     * @param neighbor_id ID of the neighboring simplex
     */
    void add_neighbor(SimplexID neighbor_id);

    /**
     * @brief Remove a neighboring simplex
     * @param neighbor_id ID of the neighbor to remove
     */
    void remove_neighbor(SimplexID neighbor_id);

    /**
     * @brief Check if the simplex is adjacent to the given simplex
     * @param neighbor_id ID of the simplex to check
     */
    bool has_neighbor(SimplexID neighbor_id) const;

private:
    SimplexID id_;
    std::vector<VertexID> vertices_;
    std::unordered_set<SimplexID> neighbors_;
};

} // namespace cebu
