#include "cebu/snapshot_manager.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <functional>
#include <algorithm>

namespace cebu {

SnapshotManager::SnapshotManager(const std::string& filename)
    : filename_(filename) {
    snapshots_.clear();
}

SnapshotMetadata SnapshotManager::create_snapshot(
    const SimplicialComplex& complex,
    const std::string& name,
    bool compress) {
    
    // Check if snapshot already exists
    if (has_snapshot(name)) {
        throw std::runtime_error("Snapshot '" + name + "' already exists");
    }
    
    // Create snapshot
    auto snapshot = std::make_shared<Snapshot>();
    
    // Serialize to JSON
    snapshot->data = JsonSerializer::serialize(complex);
    
    // Create metadata
    snapshot->metadata = create_metadata(complex, name);
    snapshot->metadata.simplex_count = complex.simplex_count();
    snapshot->metadata.vertex_count = complex.vertex_count();
    snapshot->metadata.max_dimension = complex.max_dimension();
    snapshot->metadata.size = snapshot->data.dump().size();
    
    // Compress if requested
    if (compress && Compression::has_zlib_support()) {
        compress_snapshot(*snapshot);
    }
    
    // Store snapshot
    snapshots_[name] = snapshot;
    
    return snapshot->metadata;
}

std::vector<SnapshotMetadata> SnapshotManager::list_snapshots() const {
    std::vector<SnapshotMetadata> result;
    result.reserve(snapshots_.size());
    
    for (const auto& pair : snapshots_) {
        result.push_back(pair.second->metadata);
    }
    
    return result;
}

std::shared_ptr<Snapshot> SnapshotManager::get_snapshot(const std::string& name) const {
    auto it = snapshots_.find(name);
    if (it == snapshots_.end()) {
        return nullptr;
    }
    return it->second;
}

bool SnapshotManager::delete_snapshot(const std::string& name) {
    auto it = snapshots_.find(name);
    if (it == snapshots_.end()) {
        return false;
    }
    
    snapshots_.erase(it);
    return true;
}

bool SnapshotManager::restore_snapshot(
    SimplicialComplex& target,
    const std::string& name) const {
    
    auto snapshot = get_snapshot(name);
    if (!snapshot) {
        return false;
    }
    
    try {
        // Decompress if needed
        nlohmann::json data = snapshot->data;
        if (snapshot->is_compressed) {
            Snapshot temp_snapshot = *snapshot;
            decompress_snapshot(temp_snapshot);
            data = temp_snapshot.data;
        }
        
        // Deserialize
        target = JsonSerializer::deserialize(data);
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

std::vector<Change> SnapshotManager::compare_snapshots(
    const std::string& name1,
    const std::string& name2) const {
    
    auto snapshot1 = get_snapshot(name1);
    auto snapshot2 = get_snapshot(name2);
    
    if (!snapshot1 || !snapshot2) {
        return {};
    }
    
    std::vector<Change> changes;
    
    try {
        // Get data (decompress if needed)
        nlohmann::json data1 = snapshot1->data;
        nlohmann::json data2 = snapshot2->data;
        
        if (snapshot1->is_compressed) {
            Snapshot temp = *snapshot1;
            decompress_snapshot(temp);
            data1 = temp.data;
        }
        
        if (snapshot2->is_compressed) {
            Snapshot temp = *snapshot2;
            decompress_snapshot(temp);
            data2 = temp.data;
        }
        
        // Compare simplex counts
        size_t count1 = data1["statistics"]["simplex_count"].get<size_t>();
        size_t count2 = data2["statistics"]["simplex_count"].get<size_t>();
        
        if (count1 != count2) {
            Change change(ChangeType::TOPOLOGY_CHANGED, 0);
            changes.push_back(change);
        }
        
        // Compare vertex counts
        size_t vertices1 = data1["statistics"]["vertex_count"].get<size_t>();
        size_t vertices2 = data2["statistics"]["vertex_count"].get<size_t>();
        
        if (vertices1 != vertices2) {
            Change change(ChangeType::TOPOLOGY_CHANGED, 0);
            changes.push_back(change);
        }
        
        // Compare max dimensions
        size_t dim1 = data1["statistics"]["max_dimension"].get<size_t>();
        size_t dim2 = data2["statistics"]["max_dimension"].get<size_t>();
        
        if (dim1 != dim2) {
            Change change(ChangeType::TOPOLOGY_CHANGED, 0);
            changes.push_back(change);
        }
        
        // Note: Detailed comparison would require more sophisticated logic
        // This is a simplified implementation
        
    } catch (const std::exception& e) {
        // Return empty list on error
    }
    
    return changes;
}

bool SnapshotManager::save_to_file(const std::string& filename) const {
    try {
        std::string actual_filename = filename.empty() ? filename_ : filename;
        
        nlohmann::json j;
        j["format"] = "cebu_snapshots";
        j["version"] = "0.7.2";
        j["snapshot_count"] = snapshots_.size();
        
        nlohmann::json snapshots_json = nlohmann::json::object();
        
        for (const auto& pair : snapshots_) {
            nlohmann::json snapshot_json;
            
            // Metadata
            snapshot_json["metadata"] = {
                {"name", pair.second->metadata.name},
                {"timestamp", pair.second->metadata.timestamp},
                {"hash", pair.second->metadata.hash},
                {"size", pair.second->metadata.size},
                {"compressed_size", pair.second->metadata.compressed_size},
                {"simplex_count", pair.second->metadata.simplex_count},
                {"vertex_count", pair.second->metadata.vertex_count},
                {"max_dimension", pair.second->metadata.max_dimension}
            };
            
            // Data
            snapshot_json["data"] = pair.second->data;
            snapshot_json["is_compressed"] = pair.second->is_compressed;
            
            if (pair.second->is_compressed) {
                snapshot_json["compressed_data"] = pair.second->compressed_data;
            }
            
            snapshots_json[pair.first] = snapshot_json;
        }
        
        j["snapshots"] = snapshots_json;
        
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

bool SnapshotManager::load_from_file(const std::string& filename) {
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
        if (!j.contains("format") || j["format"] != "cebu_snapshots") {
            return false;
        }
        
        // Load snapshots
        snapshots_.clear();
        
        for (const auto& [name, snapshot_json] : j["snapshots"].items()) {
            auto snapshot = std::make_shared<Snapshot>();
            
            // Metadata
            snapshot->metadata.name = snapshot_json["metadata"]["name"].get<std::string>();
            snapshot->metadata.timestamp = snapshot_json["metadata"]["timestamp"].get<std::string>();
            snapshot->metadata.hash = snapshot_json["metadata"]["hash"].get<std::string>();
            snapshot->metadata.size = snapshot_json["metadata"]["size"].get<size_t>();
            snapshot->metadata.compressed_size = snapshot_json["metadata"]["compressed_size"].get<size_t>();
            snapshot->metadata.simplex_count = snapshot_json["metadata"]["simplex_count"].get<size_t>();
            snapshot->metadata.vertex_count = snapshot_json["metadata"]["vertex_count"].get<size_t>();
            snapshot->metadata.max_dimension = snapshot_json["metadata"]["max_dimension"].get<size_t>();
            
            // Data
            snapshot->data = snapshot_json["data"];
            snapshot->is_compressed = snapshot_json["is_compressed"].get<bool>();
            
            if (snapshot_json.contains("compressed_data")) {
                snapshot->compressed_data = snapshot_json["compressed_data"].get<std::vector<uint8_t>>();
            }
            
            snapshots_[name] = snapshot;
        }
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

size_t SnapshotManager::get_snapshot_count() const {
    return snapshots_.size();
}

bool SnapshotManager::has_snapshot(const std::string& name) const {
    return snapshots_.find(name) != snapshots_.end();
}

size_t SnapshotManager::get_total_size() const {
    size_t total = 0;
    for (const auto& pair : snapshots_) {
        total += pair.second->metadata.compressed_size;
    }
    return total;
}

size_t SnapshotManager::get_total_uncompressed_size() const {
    size_t total = 0;
    for (const auto& pair : snapshots_) {
        total += pair.second->metadata.size;
    }
    return total;
}

void SnapshotManager::clear() {
    snapshots_.clear();
}

std::string SnapshotManager::compute_hash(const SimplicialComplex& complex) const {
    // Simple hash based on simplex count and vertex count
    std::hash<size_t> hasher;
    size_t h = hasher(complex.simplex_count());
    h ^= hasher(complex.vertex_count()) << 1;
    
    std::stringstream ss;
    ss << std::hex << h;
    return ss.str();
}

bool SnapshotManager::compress_snapshot(Snapshot& snapshot, int level) const {
    try {
        if (!Compression::has_zlib_support()) {
            return false;
        }
        
        // Convert JSON to string
        std::string json_str = snapshot.data.dump();
        std::vector<uint8_t> data(json_str.begin(), json_str.end());
        
        // Compress
        snapshot.compressed_data = Compression::compress(
            data, Compression::Algorithm::ZLIB, level);
        
        snapshot.metadata.compressed_size = snapshot.compressed_data.size();
        snapshot.is_compressed = true;
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool SnapshotManager::decompress_snapshot(Snapshot& snapshot) const {
    try {
        if (!snapshot.is_compressed) {
            return true;
        }
        
        // Decompress
        std::vector<uint8_t> decompressed = Compression::decompress(
            snapshot.compressed_data, Compression::Algorithm::ZLIB);
        
        // Convert to JSON
        std::string json_str(decompressed.begin(), decompressed.end());
        snapshot.data = nlohmann::json::parse(json_str);
        
        snapshot.is_compressed = false;
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

SnapshotMetadata SnapshotManager::create_metadata(
    const SimplicialComplex& complex,
    const std::string& name) const {
    
    SnapshotMetadata metadata;
    metadata.name = name;
    
    // Current timestamp
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    metadata.timestamp = ss.str();
    
    // Hash
    metadata.hash = compute_hash(complex);
    
    return metadata;
}

} // namespace cebu
