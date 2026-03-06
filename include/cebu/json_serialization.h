#pragma once

#include "cebu/simplicial_complex.h"
#include "cebu/simplicial_complex_labeled.h"
#include "cebu/simplicial_complex_narrative.h"
#include "cebu/refinement.h"
#include "cebu/simplicial_complex_non_hausdorff.h"
#include "cebu/equivalence_classes.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace cebu {

/// JSON serialization support for all complex types
/// Requires nlohmann/json library
class JsonSerializer {
public:
    /// Serialize basic simplicial complex to JSON
    static nlohmann::json serialize(const SimplicialComplex& complex);
    
    /// Deserialize basic simplicial complex from JSON
    static SimplicialComplex deserialize(const nlohmann::json& j);
    
    /// Serialize labeled simplicial complex to JSON
    template<typename LabelType>
    static nlohmann::json serialize_labeled(
        const SimplicialComplexLabeled<LabelType>& complex);
    
    /// Deserialize labeled simplicial complex from JSON
    template<typename LabelType>
    static SimplicialComplexLabeled<LabelType> deserialize_labeled(
        const nlohmann::json& j);
    
    /// Serialize narrative simplicial complex to JSON
    template<typename LabelType>
    static nlohmann::json serialize_narrative(
        const SimplicialComplexNarrative<LabelType>& complex);
    
    /// Deserialize narrative simplicial complex from JSON
    template<typename LabelType>
    static SimplicialComplexNarrative<LabelType> deserialize_narrative(
        const nlohmann::json& j);
    
    /// Serialize refinement simplicial complex to JSON
    template<typename LabelType>
    static nlohmann::json serialize_refinement(
        const SimplicialComplexRefinement<LabelType>& complex);
    
    /// Deserialize refinement simplicial complex from JSON
    template<typename LabelType>
    static SimplicialComplexRefinement<LabelType> deserialize_refinement(
        const nlohmann::json& j);
    
    /// Serialize non-Hausdorff simplicial complex to JSON
    static nlohmann::json serialize_non_hausdorff(
        const SimplicialComplexNonHausdorff& complex);

    /// Deserialize non-Hausdorff simplicial complex from JSON
    static SimplicialComplexNonHausdorff deserialize_non_hausdorff(
        const nlohmann::json& j);
    
    /// Serialize non-Hausdorff labeled simplicial complex to JSON
    template<typename LabelType>
    static nlohmann::json serialize_non_hausdorff_labeled(
        const SimplicialComplexNonHausdorffLabeled<LabelType>& complex);
    
    /// Deserialize non-Hausdorff labeled simplicial complex from JSON
    template<typename LabelType>
    static SimplicialComplexNonHausdorffLabeled<LabelType> 
    deserialize_non_hausdorff_labeled(const nlohmann::json& j);
    
    /// Serialize equivalence classes to JSON
    static nlohmann::json serialize_equivalence_classes(
        const EquivalenceClasses& eq_classes);
    
    /// Deserialize equivalence classes from JSON
    static EquivalenceClasses deserialize_equivalence_classes(
        const nlohmann::json& j);
    
    /// Serialize command history to JSON
    static nlohmann::json serialize_command_history(
        const CommandHistory& history);
    
    /// Deserialize command history from JSON
    static CommandHistory deserialize_command_history(
        const nlohmann::json& j);
    
    /// Pretty-print JSON with indentation
    static std::string pretty_print(const nlohmann::json& j, int indent = 2);
    
    /// Validate JSON structure
    static bool validate(const nlohmann::json& j);
    
    /// Get JSON schema
    static nlohmann::json get_schema();

private:
    /// Serialize a simplex to JSON
    static nlohmann::json serialize_simplex(const Simplex& simplex);

    /// Deserialize a simplex from JSON
    static Simplex deserialize_simplex(const nlohmann::json& j);

    /// Serialize label to JSON
    template<typename LabelType>
    static nlohmann::json serialize_label(const LabelType& label);

    /// Deserialize label from JSON
    template<typename LabelType>
    static LabelType deserialize_label(const nlohmann::json& j);

    /// Serialize a timeline milestone to JSON
    static nlohmann::json serialize_milestone(
        const std::pair<double, std::string>& milestone);

    /// Deserialize a timeline milestone from JSON
    static std::pair<double, std::string> deserialize_milestone(const nlohmann::json& j);

    /// Serialize an event to JSON
    static nlohmann::json serialize_event(
        const StoryEvent& event);

    /// Deserialize an event from JSON
    static StoryEvent deserialize_event(const nlohmann::json& j);
};

} // namespace cebu
