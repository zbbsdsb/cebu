#include "cebu/simplicial_complex.h"
#include <sstream>
#include <algorithm>
#include <stdexcept>

namespace cebu {

SimplicialComplex::SimplicialComplex() : next_simplex_id_(0) {}

VertexID SimplicialComplex::add_vertex() {
    VertexID new_id = static_cast<VertexID>(vertices_.size());
    vertices_.push_back(new_id);
    vertex_to_simplices_[new_id] = std::unordered_set<SimplexID>();
    return new_id;
}

SimplexID SimplicialComplex::add_edge(VertexID v1, VertexID v2) {
    return add_simplex({v1, v2});
}

SimplexID SimplicialComplex::add_triangle(VertexID v1, VertexID v2, VertexID v3) {
    return add_simplex({v1, v2, v3});
}

SimplexID SimplicialComplex::add_simplex(const std::vector<VertexID>& vertices) {
    // 检查顶点数量
    if (vertices.empty()) {
        throw std::invalid_argument("Simplex must have at least one vertex");
    }

    // 检查所有顶点是否存在
    for (VertexID v : vertices) {
        if (v >= vertices_.size()) {
            throw std::invalid_argument("Vertex does not exist");
        }
    }

    // 检查单纯形是否已存在
    std::string hash = compute_hash(vertices);
    for (const auto& [id, simplex] : simplices_) {
        if (simplex.vertices() == vertices) {
            return id;  // 返回已存在的单纯形 ID
        }
    }

    // 创建新单纯形
    SimplexID new_id = next_simplex_id_++;
    auto result = simplices_.emplace(new_id, Simplex(vertices, new_id));
    Simplex& simplex = result.first->second;

    // 更新顶点到单纯形的映射
    for (VertexID v : vertices) {
        vertex_to_simplices_[v].insert(new_id);
    }

    // 建立邻接关系：找到所有与此单纯形相邻的单纯形
    for (const auto& [id, other] : simplices_) {
        if (id == new_id) continue;

        // 两个单纯形相邻当且仅当它们的交集是一个共同的面
        // 这里简化：如果它们共享至少一个顶点，则认为相邻
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

    // 对于 k-单纯形，它的面是所有 (k-1)-单纯形
    // 即移除一个顶点后的所有组合
    if (simplex.dimension() == 0) {
        // 0-单纯形（顶点）没有面
        return result;
    }

    for (size_t i = 0; i < vertices.size(); ++i) {
        // 创建一个面：移除第 i 个顶点
        std::vector<VertexID> facet_vertices;
        for (size_t j = 0; j < vertices.size(); ++j) {
            if (j != i) {
                facet_vertices.push_back(vertices[j]);
            }
        }

        // 查找这个面是否已存在
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

} // namespace cebu
