#include "cebu/version_control.h"
#include "cebu/json_serialization.h"
#include <sstream>
#include <iomanip>
#include <chrono>
#include <fstream>

namespace cebu {

VersionControl::VersionControl(const std::string& filename)
    : head_(0),
      current_branch_("main"),
      next_version_id_(1),
      filename_(filename) {
    
    // Create main branch
    BranchInfo main_branch;
    main_branch.name = "main";
    main_branch.head_id = 0;
    main_branch.description = "Main branch";
    branches_["main"] = main_branch;
}

VersionID VersionControl::commit(
    const SimplicialComplex& complex,
    const std::string& message,
    const std::string& author) {
    
    // Serialize to JSON
    nlohmann::json j = JsonSerializer::serialize(complex);
    
    // Create version metadata
    VersionMetadata metadata;
    metadata.id = next_version_id_++;
    metadata.timestamp = generate_timestamp();
    metadata.message = message;
    metadata.author = author;
    metadata.parent_id = head_;
    metadata.branch = current_branch_;
    metadata.size = j.dump().size();
    metadata.simplex_count = complex.simplex_count();
    metadata.change_count = metadata.id - metadata.parent_id; // Simplified
    
    // Store version
    versions_[metadata.id] = metadata;
    version_data_[metadata.id] = j;
    
    // Update HEAD
    head_ = metadata.id;
    
    // Update branch HEAD
    if (branches_.find(current_branch_) != branches_.end()) {
        branches_[current_branch_].head_id = head_;
        branches_[current_branch_].timestamp = metadata.timestamp;
    }
    
    return metadata.id;
}

bool VersionControl::checkout(SimplicialComplex& target, VersionID version_id) {
    auto it = versions_.find(version_id);
    if (it == versions_.end()) {
        return false;
    }
    
    auto data_it = version_data_.find(version_id);
    if (data_it == version_data_.end()) {
        return false;
    }
    
    try {
        // Deserialize
        target = JsonSerializer::deserialize(data_it->second);
        
        // Update HEAD
        head_ = version_id;
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

std::vector<VersionMetadata> VersionControl::log(size_t limit) const {
    std::vector<VersionMetadata> result;
    
    for (const auto& pair : versions_) {
        result.push_back(pair.second);
    }
    
    // Sort by ID descending
    std::sort(result.begin(), result.end(),
        [](const VersionMetadata& a, const VersionMetadata& b) {
            return a.id > b.id;
        });
    
    // Apply limit
    if (limit > 0 && result.size() > limit) {
        result.resize(limit);
    }
    
    return result;
}

std::vector<Change> VersionControl::diff(VersionID version1, VersionID version2) const {
    auto it1 = version_data_.find(version1);
    auto it2 = version_data_.find(version2);
    
    if (it1 == version_data_.end() || it2 == version_data_.end()) {
        return {};
    }
    
    return compute_diff(it1->second, it2->second);
}

std::string VersionControl::create_branch(const std::string& name, VersionID base_id) {
    if (branches_.find(name) != branches_.end()) {
        throw std::runtime_error("Branch '" + name + "' already exists");
    }
    
    VersionID actual_base = (base_id == 0) ? head_ : base_id;
    
    BranchInfo branch;
    branch.name = name;
    branch.head_id = actual_base;
    branch.timestamp = generate_timestamp();
    branch.description = "Branch from " + std::to_string(actual_base);
    
    branches_[name] = branch;
    
    return name;
}

bool VersionControl::checkout_branch(std::string& branch_name) {
    auto it = branches_.find(branch_name);
    if (it == branches_.end()) {
        return false;
    }
    
    current_branch_ = branch_name;
    head_ = it->second.head_id;
    
    return true;
}

bool VersionControl::merge_branch(
    SimplicialComplex& target,
    const std::string& branch_name) {
    
    auto it = branches_.find(branch_name);
    if (it == branches_.end()) {
        return false;
    }
    
    VersionID branch_head = it->second.head_id;
    
    try {
        // Get branch version
        auto data_it = version_data_.find(branch_head);
        if (data_it == version_data_.end()) {
            return false;
        }
        
        // Deserialize branch version
        SimplicialComplex branch_complex = JsonSerializer::deserialize(data_it->second);
        
        // For simplicity, just replace with branch version
        // A real implementation would do proper merging
        target = branch_complex;
        
        // Update HEAD
        head_ = branch_head;
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool VersionControl::delete_branch(const std::string& name) {
    if (name == current_branch_ || name == "main") {
        return false; // Cannot delete current or main branch
    }
    
    return branches_.erase(name) > 0;
}

std::vector<BranchInfo> VersionControl::list_branches() const {
    std::vector<BranchInfo> result;
    result.reserve(branches_.size());
    
    for (const auto& pair : branches_) {
        result.push_back(pair.second);
    }
    
    return result;
}

std::string VersionControl::create_tag(
    const std::string& name,
    VersionID version_id,
    const std::string& description) {
    
    if (tags_.find(name) != tags_.end()) {
        throw std::runtime_error("Tag '" + name + "' already exists");
    }
    
    TagInfo tag;
    tag.name = name;
    tag.version_id = version_id;
    tag.timestamp = generate_timestamp();
    tag.description = description;
    
    tags_[name] = tag;
    
    return name;
}

bool VersionControl::delete_tag(const std::string& name) {
    return tags_.erase(name) > 0;
}

std::vector<TagInfo> VersionControl::list_tags() const {
    std::vector<TagInfo> result;
    result.reserve(tags_.size());
    
    for (const auto& pair : tags_) {
        result.push_back(pair.second);
    }
    
    return result;
}

bool VersionControl::revert(
    SimplicialComplex& target,
    VersionID version_id,
    bool create_commit) {
    
    auto it = versions_.find(version_id);
    if (it == versions_.end()) {
        return false;
    }
    
    auto data_it = version_data_.find(version_id);
    if (data_it == version_data_.end()) {
        return false;
    }
    
    try {
        // Deserialize target version
        target = JsonSerializer::deserialize(data_it->second);
        
        // Update HEAD
        head_ = version_id;
        
        // Create commit if requested
        if (create_commit) {
            nlohmann::json j = JsonSerializer::serialize(target);
            
            VersionMetadata metadata;
            metadata.id = next_version_id_++;
            metadata.timestamp = generate_timestamp();
            metadata.message = "Revert to version " + std::to_string(version_id);
            metadata.author = "system";
            metadata.parent_id = version_id;
            metadata.branch = current_branch_;
            metadata.size = j.dump().size();
            metadata.simplex_count = target.simplex_count();
            metadata.change_count = 0;
            
            versions_[metadata.id] = metadata;
            version_data_[metadata.id] = j;
            
            head_ = metadata.id;
            
            // Update branch HEAD
            if (branches_.find(current_branch_) != branches_.end()) {
                branches_[current_branch_].head_id = head_;
                branches_[current_branch_].timestamp = metadata.timestamp;
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

VersionID VersionControl::get_head() const {
    return head_;
}

std::string VersionControl::get_current_branch() const {
    return current_branch_;
}

SimplicialComplex VersionControl::get_version(VersionID version_id) const {
    auto data_it = version_data_.find(version_id);
    if (data_it == version_data_.end()) {
        throw std::runtime_error("Version not found");
    }
    
    return JsonSerializer::deserialize(data_it->second);
}

bool VersionControl::save_to_file(const std::string& filename) const {
    try {
        std::string actual_filename = filename.empty() ? filename_ : filename;
        
        nlohmann::json j;
        j["format"] = "cebu_versions";
        j["version"] = "0.7.2";
        j["head"] = head_;
        j["current_branch"] = current_branch_;
        j["next_version_id"] = next_version_id_;
        
        // Versions
        nlohmann::json versions_json = nlohmann::json::object();
        for (const auto& pair : versions_) {
            nlohmann::json metadata_json;
            metadata_json["id"] = pair.second.id;
            metadata_json["timestamp"] = pair.second.timestamp;
            metadata_json["message"] = pair.second.message;
            metadata_json["author"] = pair.second.author;
            metadata_json["parent_id"] = pair.second.parent_id;
            metadata_json["branch"] = pair.second.branch;
            metadata_json["size"] = pair.second.size;
            metadata_json["simplex_count"] = pair.second.simplex_count;
            metadata_json["change_count"] = pair.second.change_count;
            
            versions_json[std::to_string(pair.first)] = metadata_json;
        }
        j["versions"] = versions_json;
        
        // Version data (simplified - in production, this would use delta storage)
        nlohmann::json version_data_json = nlohmann::json::object();
        for (const auto& pair : version_data_) {
            version_data_json[std::to_string(pair.first)] = pair.second;
        }
        j["version_data"] = version_data_json;
        
        // Branches
        nlohmann::json branches_json = nlohmann::json::object();
        for (const auto& pair : branches_) {
            nlohmann::json branch_json;
            branch_json["name"] = pair.second.name;
            branch_json["head_id"] = pair.second.head_id;
            branch_json["timestamp"] = pair.second.timestamp;
            branch_json["description"] = pair.second.description;
            
            branches_json[pair.first] = branch_json;
        }
        j["branches"] = branches_json;
        
        // Tags
        nlohmann::json tags_json = nlohmann::json::object();
        for (const auto& pair : tags_) {
            nlohmann::json tag_json;
            tag_json["name"] = pair.second.name;
            tag_json["version_id"] = pair.second.version_id;
            tag_json["timestamp"] = pair.second.timestamp;
            tag_json["description"] = pair.second.description;
            
            tags_json[pair.first] = tag_json;
        }
        j["tags"] = tags_json;
        
        // Write to file
        std::ofstream out(actual_filename);
        if (!out) {
            return false;
        }
        
        out << j.dump(2);
        out.close();
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool VersionControl::load_from_file(const std::string& filename) {
    try {
        std::string actual_filename = filename.empty() ? filename_ : filename;
        
        std::ifstream in(actual_filename);
        if (!in) {
            return false;
        }
        
        nlohmann::json j;
        in >> j;
        in.close();
        
        // Validate format
        if (!j.contains("format") || j["format"] != "cebu_versions") {
            return false;
        }
        
        // Load basic info
        head_ = j["head"].get<VersionID>();
        current_branch_ = j["current_branch"].get<std::string>();
        next_version_id_ = j["next_version_id"].get<VersionID>();
        
        // Load versions
        versions_.clear();
        for (const auto& [key, value] : j["versions"].items()) {
            VersionMetadata metadata;
            metadata.id = value["id"].get<VersionID>();
            metadata.timestamp = value["timestamp"].get<std::string>();
            metadata.message = value["message"].get<std::string>();
            metadata.author = value["author"].get<std::string>();
            metadata.parent_id = value["parent_id"].get<VersionID>();
            metadata.branch = value["branch"].get<std::string>();
            metadata.size = value["size"].get<size_t>();
            metadata.simplex_count = value["simplex_count"].get<size_t>();
            metadata.change_count = value["change_count"].get<size_t>();
            
            versions_[metadata.id] = metadata;
        }
        
        // Load version data
        version_data_.clear();
        for (const auto& [key, value] : j["version_data"].items()) {
            VersionID id = std::stoull(key);
            version_data_[id] = value;
        }
        
        // Load branches
        branches_.clear();
        for (const auto& [key, value] : j["branches"].items()) {
            BranchInfo branch;
            branch.name = value["name"].get<std::string>();
            branch.head_id = value["head_id"].get<VersionID>();
            branch.timestamp = value["timestamp"].get<std::string>();
            branch.description = value["description"].get<std::string>();
            
            branches_[key] = branch;
        }
        
        // Load tags
        tags_.clear();
        for (const auto& [key, value] : j["tags"].items()) {
            TagInfo tag;
            tag.name = value["name"].get<std::string>();
            tag.version_id = value["version_id"].get<VersionID>();
            tag.timestamp = value["timestamp"].get<std::string>();
            tag.description = value["description"].get<std::string>();
            
            tags_[key] = tag;
        }
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

size_t VersionControl::get_version_count() const {
    return versions_.size();
}

size_t VersionControl::get_branch_count() const {
    return branches_.size();
}

size_t VersionControl::get_tag_count() const {
    return tags_.size();
}

std::string VersionControl::generate_timestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::vector<Change> VersionControl::compute_diff(
    const nlohmann::json& version1,
    const nlohmann::json& version2) const {
    
    std::vector<Change> changes;
    
    try {
        // Compare statistics
        size_t count1 = version1["statistics"]["simplex_count"].get<size_t>();
        size_t count2 = version2["statistics"]["simplex_count"].get<size_t>();
        
        if (count1 != count2) {
            Change change(ChangeType::TOPOLOGY_CHANGED, 0);
            changes.push_back(change);
        }
        
        size_t vertices1 = version1["statistics"]["vertex_count"].get<size_t>();
        size_t vertices2 = version2["statistics"]["vertex_count"].get<size_t>();
        
        if (vertices1 != vertices2) {
            Change change(ChangeType::TOPOLOGY_CHANGED, 0);
            changes.push_back(change);
        }
        
        // Note: Detailed comparison would require more sophisticated logic
        
    } catch (const std::exception& e) {
        // Return empty list on error
    }
    
    return changes;
}

bool VersionControl::apply_diff(
    SimplicialComplex& target,
    VersionID base_id,
    VersionID target_id) {
    
    // Simplified implementation - just checkout target_id
    return checkout(target, target_id);
}

} // namespace cebu
