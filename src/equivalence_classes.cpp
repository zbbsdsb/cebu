#include "cebu/equivalence_classes.h"
#include <stdexcept>

namespace cebu {

// ============================================================================
// EquivalenceClass Implementation
// ============================================================================

EquivalenceClass::EquivalenceClass(SimplexID representative_id)
    : representative_(representative_id) {
    members_.insert(representative_id);
}

void EquivalenceClass::add_member(SimplexID simplex_id) {
    members_.insert(simplex_id);
}

bool EquivalenceClass::remove_member(SimplexID simplex_id) {
    auto it = members_.find(simplex_id);
    if (it == members_.end()) {
        return false;
    }

    members_.erase(it);

    // If we removed the representative, pick a new one
    if (simplex_id == representative_ && !members_.empty()) {
        representative_ = *members_.begin();
    }

    return true;
}

void EquivalenceClass::set_representative(SimplexID simplex_id) {
    if (members_.find(simplex_id) == members_.end()) {
        throw std::invalid_argument("Cannot set representative to non-member simplex");
    }
    representative_ = simplex_id;
}

// ============================================================================
// EquivalenceClassManager Implementation
// ============================================================================

EquivalenceClassManager::EquivalenceClassManager() {
}

void EquivalenceClassManager::add_simplex(SimplexID simplex_id) {
    parent_[simplex_id] = simplex_id;
    sizes_[simplex_id] = 1;
    representatives_.insert(simplex_id);
}

void EquivalenceClassManager::remove_simplex(SimplexID simplex_id) {
    auto parent_it = parent_.find(simplex_id);
    if (parent_it == parent_.end()) {
        return; // Simplex not in manager
    }

    SimplexID rep = find_internal(simplex_id);

    // Remove from parent map
    parent_.erase(simplex_id);

    // Remove from sizes
    sizes_.erase(simplex_id);

    // If the simplex was a representative, update
    if (simplex_id == rep) {
        representatives_.erase(simplex_id);

        // Check if there are other members
        auto new_rep = static_cast<SimplexID>(-1);
        for (const auto& [sid, parent] : parent_) {
            if (find_internal(sid) == simplex_id && sid != simplex_id) {
                new_rep = sid;
                break;
            }
        }

        if (new_rep != static_cast<SimplexID>(-1)) {
            // Found a new representative
            representatives_.insert(new_rep);
            sizes_[new_rep] = sizes_[simplex_id] - 1;
        }
    } else {
        // Decrease the size of the class
        if (sizes_.find(rep) != sizes_.end()) {
            sizes_[rep]--;
        }
    }
}

SimplexID EquivalenceClassManager::glue(SimplexID simplex_a, SimplexID simplex_b) {
    // Add simplices if they don't exist
    if (parent_.find(simplex_a) == parent_.end()) {
        add_simplex(simplex_a);
    }
    if (parent_.find(simplex_b) == parent_.end()) {
        add_simplex(simplex_b);
    }

    // Already glued?
    SimplexID rep_a = find_internal(simplex_a);
    SimplexID rep_b = find_internal(simplex_b);

    if (rep_a == rep_b) {
        return rep_a; // Already in same class
    }

    // Union by size: attach smaller tree under larger tree
    if (sizes_[rep_a] < sizes_[rep_b]) {
        std::swap(rep_a, rep_b);
    }

    // rep_b becomes child of rep_a
    parent_[rep_b] = rep_a;
    sizes_[rep_a] += sizes_[rep_b];
    representatives_.erase(rep_b);

    return rep_a;
}

bool EquivalenceClassManager::separate(SimplexID simplex_id) {
    auto parent_it = parent_.find(simplex_id);
    if (parent_it == parent_.end()) {
        return false; // Simplex not in manager
    }

    SimplexID rep = find_internal(simplex_id);

    // If it's already alone, nothing to do
    if (rep == simplex_id) {
        return sizes_[simplex_id] == 1;
    }

    // Count other members
    size_t other_count = 0;
    for (const auto& [sid, parent] : parent_) {
        if (sid != simplex_id && find_internal(sid) == rep) {
            other_count++;
        }
    }

    if (other_count == 0) {
        // We're the only one, already effectively separated
        return false;
    }

    // Remove the simplex and re-add it
    remove_simplex(simplex_id);
    add_simplex(simplex_id);

    return true;
}

SimplexID EquivalenceClassManager::find_representative(SimplexID simplex_id) const {
    auto it = parent_.find(simplex_id);
    if (it == parent_.end()) {
        return simplex_id; // Not in manager, assume it's its own rep
    }
    return find_internal(simplex_id);
}

std::vector<SimplexID> EquivalenceClassManager::get_equivalence_class(SimplexID simplex_id) const {
    std::vector<SimplexID> result;

    auto it = parent_.find(simplex_id);
    if (it == parent_.end()) {
        result.push_back(simplex_id);
        return result;
    }

    SimplexID rep = find_internal(simplex_id);
    for (const auto& [sid, parent] : parent_) {
        if (find_internal(sid) == rep) {
            result.push_back(sid);
        }
    }

    return result;
}

bool EquivalenceClassManager::are_glued(SimplexID simplex_a, SimplexID simplex_b) const {
    // Check if either simplex is not in the manager
    bool has_a = parent_.find(simplex_a) != parent_.end();
    bool has_b = parent_.find(simplex_b) != parent_.end();

    if (!has_a && !has_b) {
        return false; // Neither is in the manager
    }
    if (!has_a || !has_b) {
        return false; // One is in, one is not - they can't be glued
    }

    return find_internal(simplex_a) == find_internal(simplex_b);
}

size_t EquivalenceClassManager::class_size(SimplexID simplex_id) const {
    auto it = parent_.find(simplex_id);
    if (it == parent_.end()) {
        return 1; // Not in manager, assume it's alone
    }

    SimplexID rep = find_internal(simplex_id);
    auto size_it = sizes_.find(rep);
    if (size_it == sizes_.end()) {
        return 1;
    }
    return size_it->second;
}

bool EquivalenceClassManager::is_glued(SimplexID simplex_id) const {
    return class_size(simplex_id) > 1;
}

std::unordered_map<SimplexID, std::unordered_set<SimplexID>> EquivalenceClassManager::get_all_classes() const {
    std::unordered_map<SimplexID, std::unordered_set<SimplexID>> result;

    for (const auto& [sid, parent] : parent_) {
        SimplexID rep = find_internal(sid);
        result[rep].insert(sid);
    }

    return result;
}

void EquivalenceClassManager::clear() {
    parent_.clear();
    representatives_.clear();
    sizes_.clear();
}

SimplexID EquivalenceClassManager::find_internal(SimplexID simplex_id) const {
    auto it = parent_.find(simplex_id);
    if (it == parent_.end()) {
        return simplex_id;
    }

    SimplexID& parent = const_cast<SimplexID&>(it->second);

    // Path compression: make every node point directly to root
    if (parent != simplex_id) {
        parent = find_internal(parent);
    }

    return parent;
}

void EquivalenceClassManager::update_representatives(SimplexID removed_rep) {
    representatives_.erase(removed_rep);
}

} // namespace cebu
