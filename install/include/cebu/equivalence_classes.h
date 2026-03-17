#pragma once

#include "cebu/simplex.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>

namespace cebu {

/**
 * @brief Represents an equivalence class of glued simplices
 *
 * In non-Hausdorff topology, simplices can be glued together meaning
 * they share the same topological identity but can have different
 * labels and attributes.
 */
class EquivalenceClass {
public:
    /**
     * @brief Constructor
     * @param representative_id The ID of the simplex representing this class
     */
    explicit EquivalenceClass(SimplexID representative_id);

    /**
     * @brief Add a simplex to this equivalence class
     * @param simplex_id ID of the simplex to add
     */
    void add_member(SimplexID simplex_id);

    /**
     * @brief Remove a simplex from this equivalence class
     * @param simplex_id ID of the simplex to remove
     * @return true if the simplex was removed, false if it wasn't a member
     */
    bool remove_member(SimplexID simplex_id);

    /**
     * @brief Get the representative simplex ID
     */
    SimplexID representative() const { return representative_; }

    /**
     * @brief Get all members of this equivalence class
     */
    const std::unordered_set<SimplexID>& members() const { return members_; }

    /**
     * @brief Get the number of members in this class
     */
    size_t size() const { return members_.size(); }

    /**
     * @brief Check if a simplex is a member of this class
     */
    bool contains(SimplexID simplex_id) const {
        return members_.find(simplex_id) != members_.end();
    }

    /**
     * @brief Set the representative simplex
     * @param simplex_id The new representative
     */
    void set_representative(SimplexID simplex_id);

private:
    SimplexID representative_;
    std::unordered_set<SimplexID> members_;
};

/**
 * @brief Manager for equivalence classes using Union-Find data structure
 *
 * This class manages the gluing relationships between simplices.
 * Uses Union-Find (Disjoint Set Union) with path compression for efficiency.
 */
class EquivalenceClassManager {
public:
    /**
     * @brief Constructor
     */
    EquivalenceClassManager();

    /**
     * @brief Add a simplex to the manager (initially in its own class)
     * @param simplex_id ID of the simplex to add
     */
    void add_simplex(SimplexID simplex_id);

    /**
     * @brief Remove a simplex from all equivalence classes
     * @param simplex_id ID of the simplex to remove
     */
    void remove_simplex(SimplexID simplex_id);

    /**
     * @brief Glue two simplices together (union their equivalence classes)
     * @param simplex_a First simplex ID
     * @param simplex_b Second simplex ID
     * @return The representative of the merged equivalence class
     */
    SimplexID glue(SimplexID simplex_a, SimplexID simplex_b);

    /**
     * @brief Separate a simplex from its equivalence class
     * @param simplex_id ID of the simplex to separate
     * @return true if separated, false if the simplex was already alone
     */
    bool separate(SimplexID simplex_id);

    /**
     * @brief Find the representative of a simplex's equivalence class
     * @param simplex_id ID of the simplex
     * @return The representative simplex ID, or simplex_id if not glued
     */
    SimplexID find_representative(SimplexID simplex_id) const;

    /**
     * @brief Get all members of the equivalence class containing the simplex
     * @param simplex_id ID of the simplex
     * @return Vector of all simplex IDs in the same class
     */
    std::vector<SimplexID> get_equivalence_class(SimplexID simplex_id) const;

    /**
     * @brief Check if two simplices are glued together
     * @param simplex_a First simplex ID
     * @param simplex_b Second simplex ID
     * @return true if they are in the same equivalence class
     */
    bool are_glued(SimplexID simplex_a, SimplexID simplex_b) const;

    /**
     * @brief Get the size of the equivalence class
     * @param simplex_id ID of a simplex in the class
     * @return Number of members in the class
     */
    size_t class_size(SimplexID simplex_id) const;

    /**
     * @brief Check if a simplex is glued to anything (class size > 1)
     * @param simplex_id ID of the simplex
     * @return true if the simplex is part of a glued group
     */
    bool is_glued(SimplexID simplex_id) const;

    /**
     * @brief Get all equivalence classes
     * @return Map of representative to all members
     */
    std::unordered_map<SimplexID, std::unordered_set<SimplexID>> get_all_classes() const;

    /**
     * @brief Get the number of equivalence classes
     */
    size_t class_count() const { return representatives_.size(); }

    /**
     * @brief Clear all equivalence classes
     */
    void clear();

private:
    // Union-Find parent pointers
    std::unordered_map<SimplexID, SimplexID> parent_;

    // Set of representatives (roots) for quick iteration
    std::unordered_set<SimplexID> representatives_;

    // Class sizes (for union by size optimization)
    std::unordered_map<SimplexID, size_t> sizes_;

    /**
     * @brief Internal find with path compression
     * @param simplex_id ID to find
     * @return The representative
     */
    SimplexID find_internal(SimplexID simplex_id) const;

    /**
     * @brief Update representatives set after union operation
     */
    void update_representatives(SimplexID removed_rep);
};

} // namespace cebu
