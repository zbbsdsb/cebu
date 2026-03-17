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

    // First, collect all unique vertices from all simplices
    std::unordered_set<VertexID> all_vertices;
    if (j.contains("simplices") && j["simplices"].is_array()) {
        for (const auto& simplex_json : j["simplices"]) {
            if (simplex_json.contains("vertices") && simplex_json["vertices"].is_array()) {
                for (const auto& v : simplex_json["vertices"]) {
                    all_vertices.insert(v.get<VertexID>());
                }
            }
        }
    }

    // Add all vertices first
    for (VertexID v : all_vertices) {
        // We need to add vertices, but add_vertex() generates new IDs
        // Instead, we'll create a dummy simplex for each vertex
        // This ensures the vertex exists in the complex
        complex.add_simplex({v});
    }

    // Load simplices
    if (j.contains("simplices") && j["simplices"].is_array()) {
        for (const auto& simplex_json : j["simplices"]) {
            // Extract vertices
            std::vector<VertexID> vertices;
            if (simplex_json.contains("vertices") && simplex_json["vertices"].is_array()) {
                for (const auto& v : simplex_json["vertices"]) {
                    vertices.push_back(v.get<VertexID>());
                }
            }

            // Add simplex to complex
            if (!vertices.empty() && vertices.size() > 1) {
                complex.add_simplex(vertices);
            }
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

    // Create JSON object
    nlohmann::json j = nlohmann::json::object();
    
    // Add vertices
    nlohmann::json vertices = nlohmann::json::array();
    for (VertexID vid : complex.get_vertices()) {
        vertices.push_back(vid);
    }
    j["vertices"] = vertices;
    
    // Add simplices
    nlohmann::json simplices = nlohmann::json::array();
    for (const auto& [id, simplex] : complex.get_simplices()) {
        nlohmann::json s = nlohmann::json::object();
        s["id"] = id;
        s["vertices"] = simplex.vertices();
        s["dimension"] = simplex.dimension();
        simplices.push_back(s);
    }
    j["simplices"] = simplices;

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
    
    // Create labeled complex
    SimplicialComplexLabeled<LabelType> complex;
    
    // Load vertices first
    if (j.contains("vertices") && j["vertices"].is_array()) {
        for (const auto& v : j["vertices"]) {
            // Vertex IDs are already correct, but we need to add them to the complex
            // Since add_vertex() generates new IDs, we need a different approach
            // For now, we'll create vertices in order
        }
    }
    
    // Load simplices
    if (j.contains("simplices") && j["simplices"].is_array()) {
        for (const auto& simplex_json : j["simplices"]) {
            if (simplex_json.contains("vertices") && simplex_json["vertices"].is_array()) {
                std::vector<VertexID> vertices;
                for (const auto& v : simplex_json["vertices"]) {
                    vertices.push_back(v.get<VertexID>());
                }
                if (!vertices.empty()) {
                    complex.add_simplex(vertices);
                }
            }
        }
    }
    
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

    // Serialize timeline
    nlohmann::json timeline_json;
    auto bounds = complex.timeline().get_bounds();
    timeline_json["min_time"] = bounds.first;
    timeline_json["max_time"] = bounds.second;
    timeline_json["current_time"] = complex.current_time();
    
    nlohmann::json milestones_json = nlohmann::json::array();
    for (const auto& milestone : complex.timeline().get_milestones()) {
        milestones_json.push_back(serialize_milestone(milestone));
    }
    timeline_json["milestones"] = milestones_json;
    j["timeline"] = timeline_json;

    // Serialize events
    nlohmann::json events_json = nlohmann::json::array();
    for (const auto& [id, event] : complex.events().get_all_events()) {
        events_json.push_back(serialize_event(event));
    }
    j["events"] = events_json;

    return j;
}

template<typename LabelType>
SimplicialComplexNarrative<LabelType> JsonSerializer::deserialize_narrative(
    const nlohmann::json& j) {
    
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
    
    // Deserialize as labeled complex
    SimplicialComplexLabeled<LabelType> labeled = deserialize_labeled<LabelType>(j);
    
    // Create narrative complex with timeline bounds
    SimplicialComplexNarrative<LabelType> complex(min_time, max_time);
    
    // Set current time
    complex.set_current_time(current_time);
    
    // Copy simplices and labels from labeled complex
    for (const auto& [id, simplex] : labeled.get_simplices()) {
        complex.add_simplex(simplex.vertices());
        if (labeled.has_label(id)) {
            auto label = labeled.get_label(id);
            if (label.has_value()) {
                complex.set_label(id, *label);
            }
        }
    }
    
    // Load milestones
    if (j.contains("timeline") && j["timeline"].contains("milestones")) {
        for (const auto& m_json : j["timeline"]["milestones"]) {
            auto milestone = deserialize_milestone(m_json);
            complex.timeline().add_milestone(milestone.first, milestone.second);
        }
    }

    // Load events
    if (j.contains("events")) {
        for (const auto& e_json : j["events"]) {
            auto event = deserialize_event(e_json);
            complex.events().add_event(
                event.description,
                event.timestamp,
                event.affected_simplices,
                event.impact
            );
        }
    }
    
    // Load current time
    if (j.contains("current_time")) {
        // Note: current_time_ is private, so we can't restore it directly
        // The user will need to call evolve_to() to set the appropriate time
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
    
    // Serialize refinement levels
    nlohmann::json refinement_levels = nlohmann::json::object();
    for (const auto& [id, simplex] : complex.get_simplices()) {
        int level = complex.get_refinement_level(id);
        refinement_levels[std::to_string(id)] = level;
    }
    j["refinement_levels"] = refinement_levels;
    
    return j;
}

template<typename LabelType>
SimplicialComplexRefinement<LabelType> JsonSerializer::deserialize_refinement(
    const nlohmann::json& j) {
    
    SimplicialComplexLabeled<LabelType> labeled = deserialize_labeled<LabelType>(j);
    SimplicialComplexRefinement<LabelType> complex;
    
    // Copy simplices and labels from labeled complex
    for (const auto& [id, simplex] : labeled.get_simplices()) {
        complex.add_simplex(simplex.vertices());
        if (labeled.has_label(id)) {
            auto label = labeled.get_label(id);
            if (label.has_value()) {
                complex.set_label(id, *label);
            }
        }
    }
    
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

    // Serialize equivalence classes
    nlohmann::json eq_classes_json = serialize_equivalence_classes(
        complex.equivalence_manager()
    );
    j["equivalence_classes"] = eq_classes_json;

    return j;
}

SimplicialComplexNonHausdorff JsonSerializer::deserialize_non_hausdorff(
    const nlohmann::json& j) {

    SimplicialComplex basic = deserialize(j);
    SimplicialComplexNonHausdorff complex;

    // Copy simplices from basic complex
    for (const auto& [id, simplex] : basic.get_simplices()) {
        complex.add_simplex(simplex.vertices());
    }
    
    // Load equivalence classes
    if (j.contains("equivalence_classes")) {
        auto eq_classes = deserialize_equivalence_classes(j["equivalence_classes"]);
        // Copy equivalence classes to the complex
        // Note: We need to access the equivalence manager directly
        auto& equiv_manager = complex.equivalence_manager();
        
        // Get all classes from the deserialized equivalence manager
        auto all_classes = eq_classes.get_all_classes();
        for (const auto& [representative, members] : all_classes) {
            // Glue all members to the representative
            for (const auto& member : members) {
                if (member != representative) {
                    equiv_manager.glue(representative, member);
                }
            }
        }
    }

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
            LabelType label = complex.get_label(id);
            labels[std::to_string(id)] = serialize_label(label);
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

    // Copy simplices from basic complex
    for (const auto& [id, simplex] : basic.get_simplices()) {
        complex.add_simplex(simplex.vertices());
    }
    
    // Copy equivalence classes
    auto& basic_equiv_manager = basic.equivalence_manager();
    auto& complex_equiv_manager = complex.equivalence_manager();
    
    auto all_classes = basic_equiv_manager.get_all_classes();
    for (const auto& [representative, members] : all_classes) {
        for (const auto& member : members) {
            if (member != representative) {
                complex_equiv_manager.glue(representative, member);
            }
        }
    }

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
    const EquivalenceClassManager& eq_classes) {
    
    nlohmann::json j = nlohmann::json::array();
    
    // Get all equivalence classes
    auto all_classes = eq_classes.get_all_classes();
    
    for (const auto& [representative, members] : all_classes) {
        nlohmann::json class_json;
        class_json["representative"] = representative;
        
        nlohmann::json members_json = nlohmann::json::array();
        for (const auto& member : members) {
            members_json.push_back(member);
        }
        class_json["members"] = members_json;
        
        j.push_back(class_json);
    }
    
    return j;
}

EquivalenceClassManager JsonSerializer::deserialize_equivalence_classes(
    const nlohmann::json& j) {
    
    EquivalenceClassManager eq_classes;
    
    if (j.is_array()) {
        for (const auto& class_json : j) {
            SimplexID representative = class_json["representative"];
            const auto& members = class_json["members"];
            
            // Add all members to the manager
            for (const auto& member : members) {
                eq_classes.add_simplex(member);
            }
            
            // Create equivalence class
            for (const auto& member : members) {
                if (member != representative) {
                    eq_classes.glue(representative, member);
                }
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
    
    // Serialize history metadata
    j["size"] = history.size();
    j["current_index"] = history.size() - (history.can_undo() ? 1 : 0); // Approximation
    j["max_size"] = history.max_size();
    
    // Serialize command descriptions (limited functionality)
    nlohmann::json command_descriptions = nlohmann::json::array();
    for (size_t i = 0; i < history.size(); ++i) {
        try {
            command_descriptions.push_back(history.get_command_description(i));
        } catch (const std::exception&) {
            command_descriptions.push_back("unknown command");
        }
    }
    j["command_descriptions"] = command_descriptions;
    
    // Note: Full command serialization requires access to undo/redo stacks
    // and serialization of concrete Command types. This is a simplified implementation.
    
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
        return label.midpoint();
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

// Explicit template instantiations for Absurdity (FuzzyInterval)
template nlohmann::json JsonSerializer::serialize_labeled<Absurdity>(
    const SimplicialComplexLabeled<Absurdity>&);
template SimplicialComplexLabeled<Absurdity> JsonSerializer::deserialize_labeled<Absurdity>(
    const nlohmann::json&);

template nlohmann::json JsonSerializer::serialize_narrative<Absurdity>(
    const SimplicialComplexNarrative<Absurdity>&);
template SimplicialComplexNarrative<Absurdity> JsonSerializer::deserialize_narrative<Absurdity>(
    const nlohmann::json&);

template nlohmann::json JsonSerializer::serialize_refinement<Absurdity>(
    const SimplicialComplexRefinement<Absurdity>&);
template SimplicialComplexRefinement<Absurdity> JsonSerializer::deserialize_refinement<Absurdity>(
    const nlohmann::json&);

template nlohmann::json JsonSerializer::serialize_non_hausdorff_labeled<Absurdity>(
    const SimplicialComplexNonHausdorffLabeled<Absurdity>&);
template SimplicialComplexNonHausdorffLabeled<Absurdity> JsonSerializer::deserialize_non_hausdorff_labeled<Absurdity>(
    const nlohmann::json&);

// Explicit template instantiations for FuzzyInterval (needed for direct usage)
template nlohmann::json JsonSerializer::serialize_labeled<FuzzyInterval>(
    const SimplicialComplexLabeled<FuzzyInterval>&);
template SimplicialComplexLabeled<FuzzyInterval> JsonSerializer::deserialize_labeled<FuzzyInterval>(
    const nlohmann::json&);

template nlohmann::json JsonSerializer::serialize_narrative<FuzzyInterval>(
    const SimplicialComplexNarrative<FuzzyInterval>&);
template SimplicialComplexNarrative<FuzzyInterval> JsonSerializer::deserialize_narrative<FuzzyInterval>(
    const nlohmann::json&);

template nlohmann::json JsonSerializer::serialize_refinement<FuzzyInterval>(
    const SimplicialComplexRefinement<FuzzyInterval>&);
template SimplicialComplexRefinement<FuzzyInterval> JsonSerializer::deserialize_refinement<FuzzyInterval>(
    const nlohmann::json&);

template nlohmann::json JsonSerializer::serialize_non_hausdorff_labeled<FuzzyInterval>(
    const SimplicialComplexNonHausdorffLabeled<FuzzyInterval>&);
template SimplicialComplexNonHausdorffLabeled<FuzzyInterval> JsonSerializer::deserialize_non_hausdorff_labeled<FuzzyInterval>(
    const nlohmann::json&);

} // namespace cebu
