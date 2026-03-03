#include "cebu/simplex.h"
#include <algorithm>
#include <stdexcept>

namespace cebu {

Simplex::Simplex(const std::vector<VertexID>& vertices, SimplexID id)
    : id_(id), vertices_(vertices) {
    // 顶点列表必须非空
    if (vertices_.empty()) {
        throw std::invalid_argument("Simplex must have at least one vertex");
    }

    // 对顶点排序，确保相同集合的顶点产生相同的表示
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
