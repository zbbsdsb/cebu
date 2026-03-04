#include "cebu/simplex.h"
#include <algorithm>
#include <stdexcept>

namespace cebu {

Simplex::Simplex(const std::vector<VertexID>& vertices, SimplexID id)
    : id_(id), vertices_(vertices) {
    // Vertex list must not be empty
    if (vertices_.empty()) {
        throw std::invalid_argument("Simplex must have at least one vertex");
    }

    // Sort vertices to ensure the same set of vertices produces the same representation
    std::sort(vertices_.begin(), vertices_.end());
}

void Simplex::add_neighbor(SimplexID neighbor_id) {
    neighbors_.insert(neighbor_id);
}

void Simplex::remove_neighbor(SimplexID neighbor_id) {
    neighbors_.erase(neighbor_id);
}

bool Simplex::has_neighbor(SimplexID neighbor_id) const {
    return neighbors_.find(neighbor_id) != neighbors_.end();
}

} // namespace cebu
