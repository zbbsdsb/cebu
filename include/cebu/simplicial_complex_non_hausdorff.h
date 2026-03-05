#pragma once

#include "cebu/simplicial_complex.h"
#include "cebu/equivalence_classes.h"
#include "cebu/label.h"
#include <memory>

namespace cebu {

/**
 * @brief Non-Hausdorff simplicial complex supporting gluing operations
 *
 * This extends SimplicialComplex with support for non-Hausdorff topology,
 * where simplices can be "glued" together, meaning they share the same
 * topological identity but can have different labels and attributes.
 *
 * Key features:
 * - Glue simplices together (equivalence classes)
 * - Separate glued simplices
 * - Query adjacency accounting for glued simplices
 * - Access all members of an equivalence class
 */
class SimplicialComplexNonHausdorff : public SimplicialComplex {
public:
    /**
     * @brief Constructor
     */
    SimplicialComplexNonHausdorff();

    /**
     * @brief Glue two simplices together
     *
     * After gluing, the two simplices are considered topologically identical.
     * They will share neighbors and can be used interchangeably in queries.
     *
     * @param simplex_a First simplex ID
     * @param simplex_b Second simplex ID
     * @return The representative of the merged equivalence class
     * @throw std::invalid_argument If simplices don't exist
     */
    SimplexID glue(SimplexID simplex_a, SimplexID simplex_b);

    /**
     * @brief Glue two vertices together (convenience method)
     * @param vertex_a First vertex ID
     * @param vertex_b Second vertex ID
     * @return The representative of the merged equivalence class
     * @throw std::invalid_argument If vertices don't exist
     */
    SimplexID glue_vertices(VertexID vertex_a, VertexID vertex_b);

    /**
     * @brief Separate a simplex from its equivalence class
     *
     * After separation, the simplex becomes independent again.
     *
     * @param simplex_id ID of the simplex to separate
     * @return true if separated, false if the simplex was already alone
     * @throw std::invalid_argument If simplex doesn't exist
     */
    bool separate(SimplexID simplex_id);

    /**
     * @brief Check if two simplices are glued together
     * @param simplex_a First simplex ID
     * @param simplex_b Second simplex ID
     * @return true if they are in the same equivalence class
     */
    bool are_glued(SimplexID simplex_a, SimplexID simplex_b) const;

    /**
     * @brief Check if a simplex is glued to any other simplex
     * @param simplex_id ID of the simplex
     * @return true if the simplex is part of a glued group
     */
    bool is_glued(SimplexID simplex_id) const;

    /**
     * @brief Get the representative of a simplex's equivalence class
     * @param simplex_id ID of the simplex
     * @return The representative simplex ID
     */
    SimplexID get_representative(SimplexID simplex_id) const;

    /**
     * @brief Get all members of the equivalence class containing a simplex
     * @param simplex_id ID of a simplex in the class
     * @return Vector of all simplex IDs in the same class
     */
    std::vector<SimplexID> get_equivalence_class(SimplexID simplex_id) const;

    /**
     * @brief Get all adjacent simplices, accounting for gluing
     *
     * Returns simplices adjacent to any member of the equivalence class.
     *
     * @param simplex_id ID of the simplex
     * @return Vector of adjacent simplex IDs
     */
    std::vector<SimplexID> get_adjacent_with_gluing(SimplexID simplex_id) const;

    /**
     * @brief Get all simplices containing a vertex, accounting for gluing
     *
     * Returns simplices containing any vertex in the equivalence class.
     *
     * @param vertex_id ID of the vertex
     * @return Vector of simplex IDs
     */
    std::vector<SimplexID> get_simplices_containing_vertex_with_gluing(VertexID vertex_id) const;

    /**
     * @brief Get the number of equivalence classes
     */
    size_t equivalence_class_count() const {
        return equiv_manager_.class_count();
    }

    /**
     * @brief Get the equivalence class manager (for advanced usage)
     */
    const EquivalenceClassManager& equivalence_manager() const {
        return equiv_manager_;
    }

    /**
     * @brief Get the equivalence class manager (for advanced usage)
     */
    EquivalenceClassManager& equivalence_manager() {
        return equiv_manager_;
    }

protected:
    /**
     * @brief Called when a simplex is removed, update equivalence classes
     */
    void on_simplex_removed(SimplexID simplex_id) override;

private:
    EquivalenceClassManager equiv_manager_;
};

/**
 * @brief Non-Hausdorff simplicial complex with labels
 *
 * Combines the gluing capabilities of SimplicialComplexNonHausdorff
 * with the labeling system of SimplicialComplexLabeled.
 */
template<typename LabelType>
class SimplicialComplexNonHausdorffLabeled : public SimplicialComplexNonHausdorff {
public:
    /**
     * @brief Set a label on a simplex
     * @param simplex_id ID of the simplex
     * @param label The label value
     * @return true if label was set, false if simplex doesn't exist
     */
    bool set_label(SimplexID simplex_id, const LabelType& label);

    /**
     * @brief Get a label from a simplex
     * @param simplex_id ID of the simplex
     * @return The label value, or default if simplex doesn't exist or has no label
     */
    LabelType get_label(SimplexID simplex_id) const;

    /**
     * @brief Get all labels from an equivalence class
     * @param simplex_id ID of a simplex in the class
     * @return Map of simplex IDs to their labels
     */
    std::unordered_map<SimplexID, LabelType> get_class_labels(SimplexID simplex_id) const;

    /**
     * @brief Check if a simplex has a label
     * @param simplex_id ID of the simplex
     * @return true if the simplex has a label
     */
    bool has_label(SimplexID simplex_id) const;

private:
    std::unordered_map<SimplexID, LabelType> labels_;
};

// Template method implementations
template<typename LabelType>
bool SimplicialComplexNonHausdorffLabeled<LabelType>::set_label(
    SimplexID simplex_id, const LabelType& label) {

    if (!has_simplex(simplex_id)) {
        return false;
    }

    labels_[simplex_id] = label;
    return true;
}

template<typename LabelType>
LabelType SimplicialComplexNonHausdorffLabeled<LabelType>::get_label(
    SimplexID simplex_id) const {

    auto it = labels_.find(simplex_id);
    if (it == labels_.end()) {
        return LabelType(); // Return default value
    }
    return it->second;
}

template<typename LabelType>
std::unordered_map<SimplexID, LabelType>
SimplicialComplexNonHausdorffLabeled<LabelType>::get_class_labels(
    SimplexID simplex_id) const {

    std::unordered_map<SimplexID, LabelType> result;
    auto members = get_equivalence_class(simplex_id);

    for (SimplexID member : members) {
        auto it = labels_.find(member);
        if (it != labels_.end()) {
            result[member] = it->second;
        }
    }

    return result;
}

template<typename LabelType>
bool SimplicialComplexNonHausdorffLabeled<LabelType>::has_label(
    SimplexID simplex_id) const {

    return labels_.find(simplex_id) != labels_.end();
}

} // namespace cebu
