#include "cebu/json_serialization.h"
#include <sstream>
#include <stdexcept>

namespace cebu {

// ============================================================================
// Basic Complex Serialization
// ============================================================================

nlohmann::json JsonSerializer::serialize(const SimplicialComplex& complex) {
    nlohmann::json j;

    // Header
    j["format"] = "cebu";
    j["version"] = "0.7.1";
    j["format_version"] = 1;

    // Statistics
    j["statistics"] = {
        {"simplex_count", complex.simplex_count()},
        {"vertex_count", complex.vertex_count()}
    };

    // Simplices
    nlohmann::json simplices = nlohmann::json::array();
    for (const auto& [id, simplex] : complex.get_simplices()) {
        simplices.push_back(serialize_simplex(simplex));
    }
    j["simplices"] = simplices;

    return j;
}

SimplicialComplex JsonSerializer::deserialize(const nlohmann::json& j) {
    // Validate format
    if (!validate(j)) {
        throw std::runtime_error("Invalid JSON format for Cebu complex");
    }
    
    SimplicialComplex complex;
    
    // Load simplices
    if (j.contains("simplices") && j["simplices"].is_array()) {
        for (const auto& simplex_json : j["simplices"]) {
            Simplex simplex = deserialize_simplex(simplex_json);
            // Re-add simplex to complex
            // Note: This is a simplified version
            // In practice, we need to handle IDs carefully
        }
    }
    
    return complex;
}

// ============================================================================
// Labeled Complex Serialization
// ============================================================================

template<typename LabelType>
nlohmann::json JsonSerializer::serialize_labeled(
    const SimplicialComplexLabeled<LabelType>& complex) {

    nlohmann::json j = serialize(static_cast<const SimplicialComplex&>(complex));

    // Add labels
    nlohmann::json labels = nlohmann::json::object();
    for (const auto& [id, simplex] : complex.get_simplices()) {
        auto label = complex.get_label(id);
        if (label.has_value()) {
            labels[std::to_string(id)] = serialize_label(*label);
        }
    }
    j["labels"] = labels;

    return j;
}

template<typename LabelType>
SimplicialComplexLabeled<LabelType> JsonSerializer::deserialize_labeled(
    const nlohmann::json& j) {
    
    // First deserialize as basic complex
    SimplicialComplex basic = deserialize(j);
    
    // Create labeled complex
    SimplicialComplexLabeled<LabelType> complex;
    
    // Copy simplices and labels
    // Note: This is a simplified implementation
    // In practice, we need to properly reconstruct the complex
    
    // Load labels
    if (j.contains("labels") && j["labels"].is_object()) {
        for (const auto& [key, value] : j["labels"].items()) {
            SimplexID id = std::stoull(key);
            LabelType label = deserialize_label<LabelType>(value);
            complex.set_label(id, label);
        }
    }
    
    return complex;
}

// ============================================================================
// Narrative Complex Serialization
// ============================================================================

template<typename LabelType>
nlohmann::json JsonSerializer::serialize_narrative(
    const SimplicialComplexNarrative<LabelType>& complex) {

    nlohmann::json j = serialize_labeled(static_cast<const SimplicialComplexLabeled<LabelType>&>(complex));

    // Add timeline - placeholder implementation
    // Timeline methods need to be verified against actual API
    j["timeline"] = nlohmann::json::object();

    // Add events - placeholder implementation
    // Event system methods need to be verified against actual API
    j["events"] = nlohmann::json::array();

    return j;
}

template<typename LabelType>
SimplicialComplexNarrative<LabelType> JsonSerializer::deserialize_narrative(
    const nlohmann::json& j) {
    
    // Deserialize as labeled complex
    SimplicialComplexLabeled<LabelType> labeled = deserialize_labeled<LabelType>(j);
    
    // Extract timeline bounds
    double min_time = 0.0;
    double max_time = 100.0;
    double current_time = 0.0;
    
    if (j.contains("timeline")) {
        const auto& timeline = j["timeline"];
        min_time = timeline.value("min_time", 0.0);
        max_time = timeline.value("max_time", 100.0);
        current_time = timeline.value("current_time", 0.0);
    }
    
    // Create narrative complex
    SimplicialComplexNarrative<LabelType> complex(min_time, max_time);
    
    // Copy from labeled complex
    // Note: Simplified implementation
    
    // Load milestones and events
    if (j.contains("timeline") && j["timeline"].contains("milestones")) {
        for (const auto& m_json : j["timeline"]["milestones"]) {
            auto milestone = deserialize_milestone(m_json);
            // Add milestone to timeline
        }
    }

    if (j.contains("events")) {
        for (const auto& e_json : j["events"]) {
            auto event = deserialize_event(e_json);
            // Add event to events system
        }
    }
    
    return complex;
}

// ============================================================================
// Refinement Complex Serialization
// ============================================================================

template<typename LabelType>
nlohmann::json JsonSerializer::serialize_refinement(
    const SimplicialComplexRefinement<LabelType>& complex) {
    
    nlohmann::json j = serialize_labeled(static_cast<const SimplicialComplexLabeled<LabelType>&>(complex));
    
    // Add refinement levels
    nlohmann::json refinement_levels = nlohmann::json::object();
    // Note: Need access to refinement_levels_ member
    // For now, this is a placeholder
    j["refinement_levels"] = refinement_levels;
    
    return j;
}

template<typename LabelType>
SimplicialComplexRefinement<LabelType> JsonSerializer::deserialize_refinement(
    const nlohmann::json& j) {
    
    SimplicialComplexLabeled<LabelType> labeled = deserialize_labeled<LabelType>(j);
    SimplicialComplexRefinement<LabelType> complex;
    
    // Copy from labeled complex
    // Note: Simplified implementation
    
    // Load refinement levels
    if (j.contains("refinement_levels")) {
        for (const auto& [key, value] : j["refinement_levels"].items()) {
            SimplexID id = std::stoull(key);
            int32_t level = value.get<int32_t>();
            complex.set_refinement_level(id, level);
        }
    }
    
    return complex;
}

// ============================================================================
// Non-Hausdorff Complex Serialization
// ============================================================================

nlohmann::json JsonSerializer::serialize_non_hausdorff(
    const SimplicialComplexNonHausdorff& complex) {

    nlohmann::json j = serialize(static_cast<const SimplicialComplex&>(complex));

    // Add equivalence classes - placeholder implementation
    j["equivalence_classes"] = nlohmann::json::object();

    return j;
}

SimplicialComplexNonHausdorff JsonSerializer::deserialize_non_hausdorff(
    const nlohmann::json& j) {

    SimplicialComplex basic = deserialize(j);
    SimplicialComplexNonHausdorff complex;

    // Copy from basic complex - placeholder implementation
    // Need proper method to copy simplices from basic to non-Hausdorff complex

    return complex;
}

// ============================================================================
// Non-Hausdorff Labeled Complex Serialization
// ============================================================================

template<typename LabelType>
nlohmann::json JsonSerializer::serialize_non_hausdorff_labeled(
    const SimplicialComplexNonHausdorffLabeled<LabelType>& complex) {

    nlohmann::json j = serialize_non_hausdorff(
        static_cast<const SimplicialComplexNonHausdorff&>(complex));

    // Add labels
    nlohmann::json labels = nlohmann::json::object();
    for (const auto& [id, simplex] : complex.get_simplices()) {
        if (complex.has_label(id)) {
            auto label_opt = complex.get_label(id);
            if (label_opt.has_value()) {
                labels[std::to_string(id)] = serialize_label(label_opt.value());
            }
        }
    }
    j["labels"] = labels;

    return j;
}

template<typename LabelType>
SimplicialComplexNonHausdorffLabeled<LabelType>
JsonSerializer::deserialize_non_hausdorff_labeled(const nlohmann::json& j) {

    SimplicialComplexNonHausdorff basic =
        deserialize_non_hausdorff(j);

    SimplicialComplexNonHausdorffLabeled<LabelType> complex;

    // Copy from basic complex - placeholder implementation
    // Need proper method to copy simplices

    // Load labels
    if (j.contains("labels") && j["labels"].is_object()) {
        for (const auto& [key, value] : j["labels"].items()) {
            SimplexID id = std::stoull(key);
            LabelType label = deserialize_label<LabelType>(value);
            complex.set_label(id, label);
        }
    }
    
    return complex;
}

// ============================================================================
// Equivalence Classes Serialization
// ============================================================================

nlohmann::json JsonSerializer::serialize_equivalence_classes(
    const EquivalenceClasses& eq_classes) {
    
    nlohmann::json j = nlohmann::json::array();
    
    // Get all equivalence classes
    // Note: This is a simplified implementation
    // In practice, we need to iterate through all classes
    
    // Example structure:
    // [
    //   {
    //     "representative": 0,
    //     "members": [0, 5, 10, 15]
    //   },
    //   {
    //     "representative": 1,
    //     "members": [1, 6, 11]
    //   }
    // ]
    
    return j;
}

EquivalenceClasses JsonSerializer::deserialize_equivalence_classes(
    const nlohmann::json& j) {
    
    EquivalenceClasses eq_classes;
    
    if (j.is_array()) {
        for (const auto& class_json : j) {
            SimplexID representative = class_json["representative"];
            const auto& members = class_json["members"];
            
            // Create equivalence class
            for (const auto& member : members) {
                eq_classes.glue(representative, member);
            }
        }
    }
    
    return eq_classes;
}

// ============================================================================
// Command History Serialization
// ============================================================================

nlohmann::json JsonSerializer::serialize_command_history(
    const CommandHistory& history) {
    
    nlohmann::json j;
    
    // Serialize undo stack
    nlohmann::json undo_stack = nlohmann::json::array();
    // Note: Need to access undo stack
    j["undo_stack"] = undo_stack;
    
    // Serialize redo stack
    nlohmann::json redo_stack = nlohmann::json::array();
    // Note: Need to access redo stack
    j["redo_stack"] = redo_stack;
    
    return j;
}

CommandHistory JsonSerializer::deserialize_command_history(
    const nlohmann::json& j) {
    
    CommandHistory history;
    
    // Load undo stack
    if (j.contains("undo_stack") && j["undo_stack"].is_array()) {
        for (const auto& cmd_json : j["undo_stack"]) {
            // Deserialize and add to undo stack
        }
    }
    
    // Load redo stack
    if (j.contains("redo_stack") && j["redo_stack"].is_array()) {
        for (const auto& cmd_json : j["redo_stack"]) {
            // Deserialize and add to redo stack
        }
    }
    
    return history;
}

// ============================================================================
// Utility Functions
// ============================================================================

std::string JsonSerializer::pretty_print(const nlohmann::json& j, int indent) {
    return j.dump(indent);
}

bool JsonSerializer::validate(const nlohmann::json& j) {
    // Check required fields
    if (!j.contains("format") || j["format"] != "cebu") {
        return false;
    }
    
    if (!j.contains("version")) {
        return false;
    }
    
    if (!j.contains("simplices")) {
        return false;
    }
    
    return true;
}

nlohmann::json JsonSerializer::get_schema() {
    nlohmann::json schema = R"(
    {
        "$schema": "http://json-schema.org/draft-07/schema#",
        "type": "object",
        "properties": {
            "format": {
                "type": "string",
                "const": "cebu"
            },
            "version": {
                "type": "string"
            },
            "format_version": {
                "type": "integer"
            },
            "statistics": {
                "type": "object",
                "properties": {
                    "simplex_count": {"type": "integer"},
                    "vertex_count": {"type": "integer"},
                    "edge_count": {"type": "integer"},
                    "face_count": {"type": "integer"},
                    "max_dimension": {"type": "integer"}
                }
            },
            "simplices": {
                "type": "array",
                "items": {
                    "type": "object",
                    "properties": {
                        "id": {"type": "integer"},
                        "dimension": {"type": "integer"},
                        "vertices": {
                            "type": "array",
                            "items": {"type": "integer"}
                        }
                    }
                }
            },
            "labels": {
                "type": "object"
            },
            "equivalence_classes": {
                "type": "array"
            },
            "command_history": {
                "type": "object"
            }
        },
        "required": ["format", "version", "simplices"]
    }
    )"_json;
    
    return schema;
}

// ============================================================================
// Helper Functions
// ============================================================================

nlohmann::json JsonSerializer::serialize_simplex(const Simplex& simplex) {
    nlohmann::json j;
    j["id"] = simplex.id();
    j["dimension"] = simplex.dimension();
    
    // Serialize vertices
    nlohmann::json vertices = nlohmann::json::array();
    for (VertexID vid : simplex.vertices()) {
        vertices.push_back(vid);
    }
    j["vertices"] = vertices;
    
    return j;
}

Simplex JsonSerializer::deserialize_simplex(const nlohmann::json& j) {
    std::vector<VertexID> vertices;

    if (j.contains("vertices") && j["vertices"].is_array()) {
        for (const auto& v : j["vertices"]) {
            vertices.push_back(v.get<VertexID>());
        }
    }

    // Create simplex - note: id will be assigned when adding to complex
    // This is a placeholder - proper implementation requires complex integration
    SimplexID id = j.value("id", static_cast<SimplexID>(0));

    return Simplex(vertices, id);
}

template<typename LabelType>
nlohmann::json JsonSerializer::serialize_label(const LabelType& label) {
    // Handle different label types
    if constexpr (std::is_same_v<LabelType, double>) {
        return label;
    } else if constexpr (std::is_same_v<LabelType, float>) {
        return label;
    } else if constexpr (std::is_integral_v<LabelType>) {
        return label;
    } else if constexpr (std::is_same_v<LabelType, std::string>) {
        return label;
    } else if constexpr (std::is_same_v<LabelType, Absurdity>) {
        return label.value();
    } else {
        // For custom types, try to convert to string
        std::stringstream ss;
        ss << label;
        return ss.str();
    }
}

template<typename LabelType>
LabelType JsonSerializer::deserialize_label(const nlohmann::json& j) {
    if constexpr (std::is_same_v<LabelType, double>) {
        return j.get<double>();
    } else if constexpr (std::is_same_v<LabelType, float>) {
        return j.get<float>();
    } else if constexpr (std::is_integral_v<LabelType>) {
        return j.get<LabelType>();
    } else if constexpr (std::is_same_v<LabelType, std::string>) {
        return j.get<std::string>();
    } else if constexpr (std::is_same_v<LabelType, Absurdity>) {
        return Absurdity(j.get<double>());
    } else {
        throw std::runtime_error("Unsupported label type for deserialization");
    }
}

nlohmann::json JsonSerializer::serialize_milestone(
    const std::pair<double, std::string>& milestone) {

    nlohmann::json j;
    j["time"] = milestone.first;
    j["description"] = milestone.second;

    return j;
}

std::pair<double, std::string> JsonSerializer::deserialize_milestone(
    const nlohmann::json& j) {

    double time = j["time"];
    std::string description = j.value("description", "");

    return {time, description};
}

nlohmann::json JsonSerializer::serialize_event(
    const StoryEvent& event) {

    nlohmann::json j;
    j["id"] = event.id;
    j["description"] = event.description;
    j["timestamp"] = event.timestamp;
    j["affected_simplices"] = event.affected_simplices;

    return j;
}

StoryEvent JsonSerializer::deserialize_event(
    const nlohmann::json& j) {

    EventID id = j.value("id", static_cast<EventID>(0));
    std::string description = j.value("description", "");
    double timestamp = j.value("timestamp", 0.0);
    std::vector<SimplexID> affected_simplices;

    if (j.contains("affected_simplices") && j["affected_simplices"].is_array()) {
        for (const auto& v : j["affected_simplices"]) {
            affected_simplices.push_back(v.get<SimplexID>());
        }
    }

    // AbsurdityContext deserialization is complex - placeholder
    AbsurdityContext impact;

    return StoryEvent(id, description, timestamp, affected_simplices, impact);
}

// Explicit template instantiations
template nlohmann::json JsonSerializer::serialize_labeled<double>(
    const SimplicialComplexLabeled<double>&);
template SimplicialComplexLabeled<double> JsonSerializer::deserialize_labeled<double>(
    const nlohmann::json&);

template nlohmann::json JsonSerializer::serialize_narrative<double>(
    const SimplicialComplexNarrative<double>&);
template SimplicialComplexNarrative<double> JsonSerializer::deserialize_narrative<double>(
    const nlohmann::json&);

template nlohmann::json JsonSerializer::serialize_refinement<double>(
    const SimplicialComplexRefinement<double>&);
template SimplicialComplexRefinement<double> JsonSerializer::deserialize_refinement<double>(
    const nlohmann::json&);

template nlohmann::json JsonSerializer::serialize_non_hausdorff_labeled<double>(
    const SimplicialComplexNonHausdorffLabeled<double>&);
template SimplicialComplexNonHausdorffLabeled<double> JsonSerializer::deserialize_non_hausdorff_labeled<double>(
    const nlohmann::json&);

} // namespace cebu
