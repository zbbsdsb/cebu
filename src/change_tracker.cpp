#include "cebu/change_tracker.h"
#include "cebu/compression.h"
#include <fstream>
#include <sstream>

namespace cebu {

ChangeTracker::ChangeTracker() {
    changes_.reserve(1000);
}

void ChangeTracker::track_simplex_added(
    SimplexID simplex_id,
    size_t dimension,
    const std::vector<VertexID>& vertices) {
    
    Change change(ChangeType::SIMPLEX_ADDED, simplex_id);
    change.dimension = dimension;
    change.vertices = vertices;
    
    add_change(change);
}

void ChangeTracker::track_simplex_removed(
    SimplexID simplex_id,
    size_t dimension,
    const std::vector<VertexID>& vertices) {
    
    Change change(ChangeType::SIMPLEX_REMOVED, simplex_id);
    change.dimension = dimension;
    change.vertices = vertices;
    
    add_change(change);
}

void ChangeTracker::track_label_changed(
    SimplexID simplex_id,
    double old_label,
    double new_label) {
    
    Change change(ChangeType::LABEL_CHANGED, simplex_id);
    change.old_label = old_label;
    change.new_label = new_label;
    
    add_change(change);
}

void ChangeTracker::track_equivalence_changed(
    SimplexID equivalence_id,
    SimplexID old_equivalence,
    SimplexID new_equivalence) {

    Change change(ChangeType::EQUIVALENCE_CHANGED, equivalence_id);
    change.old_equivalence = old_equivalence;
    change.new_equivalence = new_equivalence;

    add_change(change);
}

void ChangeTracker::track_topology_changed() {
    Change change(ChangeType::TOPOLOGY_CHANGED, 0);
    add_change(change);
}

void ChangeTracker::reset() {
    changes_.clear();
    changes_.reserve(1000);
}

const std::vector<Change>& ChangeTracker::get_changes() const {
    return changes_;
}

std::vector<Change> ChangeTracker::get_changes_by_type(ChangeType type) const {
    std::vector<Change> result;
    result.reserve(changes_.size() / 4); // Estimate
    
    for (const auto& change : changes_) {
        if (change.type == type) {
            result.push_back(change);
        }
    }
    
    return result;
}

std::vector<Change> ChangeTracker::get_changes_by_simplex(SimplexID simplex_id) const {
    std::vector<Change> result;
    result.reserve(10); // Estimate
    
    for (const auto& change : changes_) {
        if (change.simplex_id == simplex_id) {
            result.push_back(change);
        }
    }
    
    return result;
}

size_t ChangeTracker::get_change_count() const {
    return changes_.size();
}

bool ChangeTracker::has_changes() const {
    return !changes_.empty();
}

std::vector<Change> ChangeTracker::get_changes_in_time_range(
    const std::chrono::system_clock::time_point& start,
    const std::chrono::system_clock::time_point& end) const {
    
    std::vector<Change> result;
    result.reserve(changes_.size() / 2); // Estimate
    
    for (const auto& change : changes_) {
        if (change.timestamp >= start && change.timestamp <= end) {
            result.push_back(change);
        }
    }
    
    return result;
}

nlohmann::json ChangeTracker::to_json() const {
    nlohmann::json j;
    j["format"] = "cebu_delta";
    j["version"] = "0.7.2";
    j["change_count"] = changes_.size();
    
    // Convert timestamp to string
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    j["timestamp"] = std::ctime(&time_t);
    
    nlohmann::json changes_json = nlohmann::json::array();
    
    for (const auto& change : changes_) {
        nlohmann::json change_json;
        
        // Convert change type to string
        switch (change.type) {
            case ChangeType::SIMPLEX_ADDED:
                change_json["type"] = "added";
                break;
            case ChangeType::SIMPLEX_REMOVED:
                change_json["type"] = "removed";
                break;
            case ChangeType::LABEL_CHANGED:
                change_json["type"] = "label_changed";
                break;
            case ChangeType::EQUIVALENCE_CHANGED:
                change_json["type"] = "equivalence_changed";
                break;
            case ChangeType::TOPOLOGY_CHANGED:
                change_json["type"] = "topology_changed";
                break;
        }
        
        change_json["simplex_id"] = change.simplex_id;
        change_json["dimension"] = change.dimension;
        change_json["vertices"] = change.vertices;
        
        auto change_time = std::chrono::system_clock::to_time_t(change.timestamp);
        change_json["timestamp"] = std::ctime(&change_time);
        
        if (change.type == ChangeType::LABEL_CHANGED) {
            change_json["old_label"] = change.old_label;
            change_json["new_label"] = change.new_label;
        }
        
        if (change.type == ChangeType::EQUIVALENCE_CHANGED) {
            change_json["old_equivalence"] = change.old_equivalence;
            change_json["new_equivalence"] = change.new_equivalence;
        }
        
        changes_json.push_back(change_json);
    }
    
    j["changes"] = changes_json;
    
    return j;
}

void ChangeTracker::from_json(const nlohmann::json& j) {
    // Validate format
    if (!j.contains("format") || j["format"] != "cebu_delta") {
        throw std::runtime_error("Invalid delta format");
    }
    
    if (!j.contains("version")) {
        throw std::runtime_error("Missing version field");
    }
    
    if (!j.contains("changes")) {
        throw std::runtime_error("Missing changes field");
    }
    
    // Clear existing changes
    changes_.clear();
    
    // Parse changes
    for (const auto& change_json : j["changes"]) {
        ChangeType type;
        
        std::string type_str = change_json["type"].get<std::string>();
        if (type_str == "added") {
            type = ChangeType::SIMPLEX_ADDED;
        } else if (type_str == "removed") {
            type = ChangeType::SIMPLEX_REMOVED;
        } else if (type_str == "label_changed") {
            type = ChangeType::LABEL_CHANGED;
        } else if (type_str == "equivalence_changed") {
            type = ChangeType::EQUIVALENCE_CHANGED;
        } else if (type_str == "topology_changed") {
            type = ChangeType::TOPOLOGY_CHANGED;
        } else {
            continue; // Skip unknown types
        }
        
        Change change(type, change_json["simplex_id"].get<SimplexID>());
        change.dimension = change_json["dimension"].get<size_t>();
        
        if (change_json.contains("vertices")) {
            change.vertices = change_json["vertices"].get<std::vector<VertexID>>();
        }
        
        if (type == ChangeType::LABEL_CHANGED) {
            change.old_label = change_json["old_label"].get<double>();
            change.new_label = change_json["new_label"].get<double>();
        }
        
        if (type == ChangeType::EQUIVALENCE_CHANGED) {
            change.old_equivalence = change_json["old_equivalence"].get<SimplexID>();
            change.new_equivalence = change_json["new_equivalence"].get<SimplexID>();
        }
        
        changes_.push_back(change);
    }
}

bool ChangeTracker::save_to_file(const std::string& filename) const {
    try {
        nlohmann::json j = to_json();
        std::string json_str = j.dump(2);
        
        // Write to file
        std::ofstream out(filename);
        if (!out) {
            return false;
        }
        
        out << json_str;
        out.close();
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool ChangeTracker::load_from_file(const std::string& filename) {
    try {
        // Read from file
        std::ifstream in(filename);
        if (!in) {
            return false;
        }
        
        nlohmann::json j;
        in >> j;
        in.close();
        
        from_json(j);
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

void ChangeTracker::add_change(const Change& change) {
    changes_.push_back(change);
}

} // namespace cebu
