#include "cebu/topology_morph.h"
#include "cebu/topology_operations.h"
#include <algorithm>
#include <random>
#include <cmath>
#include <sstream>
#include <stdexcept>

namespace cebu {

// =============================================================================
// TopologyMorph Implementation
// =============================================================================

TopologyMorph::TopologyMorph() : enabled_(true) {
    // Initialize with default rules
    auto defaults = MorphPresets::default_rules();
    for (const auto& rule : defaults) {
        add_rule(rule);
    }
}

void TopologyMorph::add_rule(const MorphRule& rule) {
    rules_.push_back(rule);
    
    // Sort by priority (higher priority first)
    std::sort(rules_.begin(), rules_.end(),
        [](const MorphRule& a, const MorphRule& b) {
            return a.priority > b.priority;
        });
}

bool TopologyMorph::remove_rule(const std::string& name) {
    auto it = std::remove_if(rules_.begin(), rules_.end(),
        [&name](const MorphRule& r) { return r.name == name; });
    
    if (it == rules_.end()) {
        return false;
    }
    
    rules_.erase(it, rules_.end());
    return true;
}

bool TopologyMorph::rule_matches(
    const MorphRule& rule,
    const FuzzyInterval& absurdity,
    const MorphContext& context,
    SimplexID simplex_id
) const {
    // Check max applications
    if (rule.max_applications > 0 && 
        rule.application_count >= rule.max_applications) {
        return false;
    }
    
    // Check probability
    static std::mt19937& rng = []() -> std::mt19937& {
        static thread_local std::mt19937 gen(std::random_device{}());
        return gen;
    }();
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    if (dist(rng) > rule.probability) {
        return false;
    }
    
    // Check trigger condition
    switch (rule.condition) {
        case TriggerCondition::ABSURDITY_THRESHOLD:
            return absurdity > rule.threshold;
            
        case TriggerCondition::LOW_ABSURDITY:
            return absurdity < rule.threshold;
            
        case TriggerCondition::USER_INTERACTION:
            // Check if simplex is in interaction region
            // This would need to be handled externally
            return false;
            
        case TriggerCondition::TEMPORAL_EVENT:
            // Check if current time matches event
            // This would need to be handled externally
            return false;
            
        case TriggerCondition::NEIGHBOR_DIFFERENCE: {
            // Compare with neighbors
            auto neighbors = context.complex.get_neighbors(simplex_id);
            for (auto neighbor_id : neighbors) {
                auto neighbor_absurdity = 
                    context.absurdity_field.get(neighbor_id);
                double diff = (absurdity - neighbor_absurdity).midpoint();
                if (std::abs(diff) > rule.threshold.midpoint()) {
                    return true;
                }
            }
            return false;
        }
            
        case TriggerCondition::GLOBAL_THRESHOLD:
            return context.global_absurdity > rule.threshold;
            
        case TriggerCondition::CUSTOM:
            if (rule.custom_trigger) {
                return rule.custom_trigger(absurdity, simplex_id);
            }
            return false;
            
        default:
            return false;
    }
}

const MorphRule* TopologyMorph::find_matching_rule(
    SimplexID simplex_id,
    const MorphContext& context
) const {
    auto absurdity = context.absurdity_field.get(simplex_id);
    
    for (const auto& rule : rules_) {
        if (rule_matches(rule, absurdity, context, simplex_id)) {
            return &rule;
        }
    }
    
    return nullptr;
}

MorphResult TopologyMorph::apply_morph(
    const MorphRule& rule,
    SimplexID simplex_id,
    MorphContext& context
) {
    MorphResult result;
    result.absurdity_before = 
        context.absurdity_field.get(simplex_id).midpoint();
    
    // Check if simplex still exists
    if (!context.complex.has_simplex(simplex_id)) {
        result.success = false;
        result.message = "Simplex no longer exists";
        return result;
    }
    
    // Apply morph based on type
    switch (rule.type) {
        case MorphType::SPLIT:
            result = split_simplex(
                context.complex, simplex_id, context.absurdity_field);
            break;
            
        case MorphType::MERGE:
            result = merge_simplices(
                context.complex, simplex_id, context.absurdity_field);
            break;
            
        case MorphType::DELETE:
            result = delete_simplex(
                context.complex, simplex_id, context.absurdity_field);
            break;
            
        case MorphType::CREATE:
            result = create_simplex(
                context.complex, simplex_id, context.absurdity_field);
            break;
            
        case MorphType::EXPAND:
            result = expand_dimension(
                context.complex, simplex_id, context.absurdity_field);
            break;
            
        case MorphType::CONTRACT:
            result = contract_dimension(
                context.complex, simplex_id, context.absurdity_field);
            break;
            
        case MorphType::GLUE:
            result = glue_vertices(
                context.complex, simplex_id, context.absurdity_field);
            break;
            
        case MorphType::REFINE:
            result = refine_simplex(
                context.complex, simplex_id, context.absurdity_field);
            break;
            
        case MorphType::COARSEN:
            result = coarsen_simplex(
                context.complex, simplex_id, context.absurdity_field);
            break;
            
        case MorphType::WRAP:
            result = wrap_boundary(
                context.complex, simplex_id, context.absurdity_field);
            break;
            
        default:
            result.success = false;
            result.message = "Unknown morph type";
            break;
    }
    
    // Use custom morph if provided
    if (rule.custom_morph && rule.custom_morph(context.complex, simplex_id)) {
        result.success = true;
        result.message = "Custom morph applied";
    }
    
    if (result.success) {
        result.absurdity_after = 
            context.absurdity_field.get(simplex_id).midpoint();
    }
    
    update_stats(result, rule.type);
    
    return result;
}

std::vector<MorphResult> TopologyMorph::apply_rules(
    MorphContext& context,
    size_t max_operations
) {
    std::vector<MorphResult> results;
    
    if (!enabled_) {
        return results;
    }
    
    size_t operations = 0;
    
    // Iterate over all simplices
    for (const auto& [id, simplex] : context.complex.get_simplices()) {
        if (max_operations > 0 && operations >= max_operations) {
            break;
        }
        
        const MorphRule* rule = find_matching_rule(id, context);
        if (rule) {
            auto result = apply_morph(*rule, id, context);
            results.push_back(result);
            
            if (result.success) {
                operations++;
                const_cast<MorphRule*>(rule)->application_count++;
            }
        }
    }
    
    return results;
}

void TopologyMorph::update_stats(const MorphResult& result, MorphType type) {
    stats_.total_operations++;
    
    if (result.success) {
        stats_.successful_operations++;
        stats_.operation_counts[type]++;
        stats_.total_absurdity_change += 
            std::abs(result.absurdity_after - result.absurdity_before);
    } else {
        stats_.failed_operations++;
    }
}

// =============================================================================
// Built-in Morph Operations
// =============================================================================

MorphResult TopologyMorph::split_simplex(
    SimplicialComplex& complex,
    SimplexID simplex_id,
    const AbsurdityField& field,
    size_t pieces
) {
    MorphResult result;
    result.success = false;
    
    // Get simplex
    if (!complex.has_simplex(simplex_id)) {
        result.message = "Simplex does not exist";
        return result;
    }
    
    const auto& simplex = complex.get_simplex(simplex_id);
    size_t dim = simplex.dimension();
    
    if (pieces < 2 || pieces > 10) {
        result.message = "Invalid number of pieces";
        return result;
    }
    
    // Only split simplices of dimension >= 1
    if (dim == 0) {
        result.message = "Cannot split vertex";
        return result;
    }
    
    result.created.reserve(pieces);
    
    // For edges: split by inserting midpoint
    if (dim == 1) {
        auto vertices = simplex.vertices();
        if (vertices.size() != 2) {
            result.message = "Invalid edge";
            return result;
        }
        
        // Remove original edge
        complex.remove_simplex(simplex_id, false);
        result.removed.push_back(simplex_id);
        
        // Create new vertex at midpoint
        auto mid_vertex = complex.add_vertex();
        result.created.push_back(mid_vertex);
        
        // Create two new edges
        auto e1 = complex.add_edge(vertices[0], mid_vertex);
        auto e2 = complex.add_edge(mid_vertex, vertices[1]);
        result.created.push_back(e1);
        result.created.push_back(e2);
        
        result.success = true;
        result.message = "Split edge into two edges";
    }
    // For triangles: split by inserting centroid
    else if (dim == 2) {
        auto vertices = simplex.vertices();
        if (vertices.size() != 3) {
            result.message = "Invalid triangle";
            return result;
        }
        
        // Remove original triangle
        complex.remove_simplex(simplex_id, false);
        result.removed.push_back(simplex_id);
        
        // Create centroid vertex
        auto centroid = complex.add_vertex();
        result.created.push_back(centroid);
        
        // Create three new triangles
        auto t1 = complex.add_triangle(vertices[0], vertices[1], centroid);
        auto t2 = complex.add_triangle(vertices[1], vertices[2], centroid);
        auto t3 = complex.add_triangle(vertices[2], vertices[0], centroid);
        result.created.push_back(t1);
        result.created.push_back(t2);
        result.created.push_back(t3);
        
        result.success = true;
        result.message = "Split triangle into three triangles";
    }
    // For higher dimensions: split by inserting centroid
    else {
        auto vertices = simplex.vertices();
        
        // Remove original simplex
        complex.remove_simplex(simplex_id, false);
        result.removed.push_back(simplex_id);
        
        // Create centroid vertex
        auto centroid = complex.add_vertex();
        result.created.push_back(centroid);
        
        // Create new simplices by replacing each vertex with centroid
        for (size_t i = 0; i < vertices.size(); ++i) {
            std::vector<VertexID> new_vertices = vertices;
            new_vertices[i] = centroid;
            auto new_simplex = complex.add_simplex(new_vertices);
            result.created.push_back(new_simplex);
        }
        
        result.success = true;
        result.message = "Split simplex into " + std::to_string(vertices.size()) + " pieces";
    }
    
    // Propagate absurdity to new simplices
    auto absurdity = field.get(simplex_id);
    for (auto new_id : result.created) {
        field.set(new_id, absurdity);
    }
    
    return result;
}

MorphResult TopologyMorph::merge_simplices(
    SimplicialComplex& complex,
    SimplexID simplex_id,
    const AbsurdityField& field
) {
    MorphResult result;
    result.success = false;
    
    if (!complex.has_simplex(simplex_id)) {
        result.message = "Simplex does not exist";
        return result;
    }
    
    // Get neighbors
    auto neighbors = complex.get_neighbors(simplex_id);
    if (neighbors.empty()) {
        result.message = "No neighbors to merge with";
        return result;
    }
    
    // Get first neighbor (simplistic approach)
    auto neighbor_id = neighbors[0];
    
    // Merge by deleting both and creating union
    complex.remove_simplex(simplex_id, false);
    complex.remove_simplex(neighbor_id, false);
    result.removed.push_back(simplex_id);
    result.removed.push_back(neighbor_id);
    
    // Combine vertices (simplified)
    std::vector<VertexID> combined_vertices;
    const auto& s1 = complex.get_simplex(simplex_id);
    const auto& s2 = complex.get_simplex(neighbor_id);
    
    for (auto v : s1.vertices()) {
        combined_vertices.push_back(v);
    }
    for (auto v : s2.vertices()) {
        combined_vertices.push_back(v);
    }
    
    // Remove duplicates
    std::sort(combined_vertices.begin(), combined_vertices.end());
    combined_vertices.erase(
        std::unique(combined_vertices.begin(), combined_vertices.end()),
        combined_vertices.end()
    );
    
    // Create merged simplex
    auto merged = complex.add_simplex(combined_vertices);
    result.created.push_back(merged);
    
    // Fuse absurdities
    auto a1 = field.get(simplex_id);
    auto a2 = field.get(neighbor_id);
    auto fused = FusionStrategy::fuse({a1, a2}, {0.5, 0.5});
    field.set(merged, fused);
    
    result.success = true;
    result.message = "Merged two simplices";
    
    return result;
}

MorphResult TopologyMorph::delete_simplex(
    SimplicialComplex& complex,
    SimplexID simplex_id,
    const AbsurdityField& field
) {
    MorphResult result;
    
    if (!complex.has_simplex(simplex_id)) {
        result.success = false;
        result.message = "Simplex does not exist";
        return result;
    }
    
    // Get faces to cascade delete
    auto faces = complex.get_faces(simplex_id);
    
    // Remove simplex and dependents
    complex.remove_simplex(simplex_id, true);
    result.removed.push_back(simplex_id);
    for (auto face_id : faces) {
        result.removed.push_back(face_id);
    }
    
    result.success = true;
    result.message = "Deleted simplex with dependents";
    
    return result;
}

MorphResult TopologyMorph::create_simplex(
    SimplicialComplex& complex,
    SimplexID simplex_id,
    const AbsurdityField& field,
    size_t dimension
) {
    MorphResult result;
    
    // Create new vertices
    std::vector<VertexID> vertices;
    for (size_t i = 0; i <= dimension; ++i) {
        auto v = complex.add_vertex();
        vertices.push_back(v);
        result.created.push_back(v);
    }
    
    // Create simplex
    auto new_simplex = complex.add_simplex(vertices);
    result.created.push_back(new_simplex);
    
    // Inherit absurdity
    auto absurdity = field.get(simplex_id);
    field.set(new_simplex, absurdity);
    
    result.success = true;
    result.message = "Created " + std::to_string(dimension) + "-simplex";
    
    return result;
}

MorphResult TopologyMorph::expand_dimension(
    SimplicialComplex& complex,
    SimplexID simplex_id,
    const AbsurdityField& field
) {
    MorphResult result;
    result.success = false;
    
    if (!complex.has_simplex(simplex_id)) {
        result.message = "Simplex does not exist";
        return result;
    }
    
    const auto& simplex = complex.get_simplex(simplex_id);
    size_t dim = simplex.dimension();
    
    // Add new vertex
    auto new_vertex = complex.add_vertex();
    result.created.push_back(new_vertex);
    
    // Get vertices
    auto vertices = simplex.vertices();
    vertices.push_back(new_vertex);
    
    // Create higher-dimensional simplex
    auto expanded = complex.add_simplex(vertices);
    result.created.push_back(expanded);
    
    // Propagate absurdity
    auto absurdity = field.get(simplex_id);
    field.set(expanded, absurdity);
    
    result.success = true;
    result.message = "Expanded to dimension " + std::to_string(dim + 1);
    
    return result;
}

MorphResult TopologyMorph::contract_dimension(
    SimplicialComplex& complex,
    SimplexID simplex_id,
    const AbsurdityField& field
) {
    MorphResult result;
    result.success = false;
    
    if (!complex.has_simplex(simplex_id)) {
        result.message = "Simplex does not exist";
        return result;
    }
    
    const auto& simplex = complex.get_simplex(simplex_id);
    size_t dim = simplex.dimension();
    
    if (dim == 0) {
        result.message = "Cannot contract vertex";
        return result;
    }
    
    // Remove simplex
    complex.remove_simplex(simplex_id, false);
    result.removed.push_back(simplex_id);
    
    // Create lower-dimensional simplex (remove last vertex)
    auto vertices = simplex.vertices();
    vertices.pop_back();
    
    auto contracted = complex.add_simplex(vertices);
    result.created.push_back(contracted);
    
    // Propagate absurdity
    auto absurdity = field.get(simplex_id);
    field.set(contracted, absurdity);
    
    result.success = true;
    result.message = "Contracted to dimension " + std::to_string(dim - 1);
    
    return result;
}

MorphResult TopologyMorph::glue_vertices(
    SimplicialComplex& complex,
    SimplexID simplex_id,
    const AbsurdityField& field
) {
    MorphResult result;
    result.success = false;
    
    if (!complex.has_simplex(simplex_id)) {
        result.message = "Simplex does not exist";
        return result;
    }
    
    const auto& simplex = complex.get_simplex(simplex_id);
    auto vertices = simplex.vertices();
    
    if (vertices.size() < 2) {
        result.message = "Need at least 2 vertices to glue";
        return result;
    }
    
    // Glue first two vertices
    auto v1 = vertices[0];
    auto v2 = vertices[1];
    
    TopologyOperations::glue_vertices(complex, v1, v2);
    result.modified.push_back(simplex_id);
    
    result.success = true;
    result.message = "Glued two vertices";
    
    return result;
}

MorphResult TopologyMorph::refine_simplex(
    SimplicialComplex& complex,
    SimplexID simplex_id,
    const AbsurdityField& field
) {
    MorphResult result;
    
    // Refine is similar to split but more controlled
    return split_simplex(complex, simplex_id, field, 2);
}

MorphResult TopologyMorph::coarsen_simplex(
    SimplicialComplex& complex,
    SimplexID simplex_id,
    const AbsurdityField& field
) {
    MorphResult result;
    result.success = false;
    
    if (!complex.has_simplex(simplex_id)) {
        result.message = "Simplex does not exist";
        return result;
    }
    
    const auto& simplex = complex.get_simplex(simplex_id);
    
    // Only coarsen simplices with interior vertices
    if (simplex.dimension() < 2) {
        result.message = "Cannot coarsen " + std::to_string(simplex.dimension()) + "-simplex";
        return result;
    }
    
    // Remove interior vertex by contracting
    return contract_dimension(complex, simplex_id, field);
}

MorphResult TopologyMorph::wrap_boundary(
    SimplicialComplex& complex,
    SimplexID simplex_id,
    const AbsurdityField& field
) {
    MorphResult result;
    result.success = false;
    
    // Find boundary edges
    auto boundary = TopologyOperations::compute_boundary(complex);
    
    if (boundary.empty()) {
        result.message = "Complex has no boundary to wrap";
        return result;
    }
    
    // Glue opposite boundary edges to create non-orientable
    // Simplified: glue first half to second half in reverse
    size_t n = boundary.size();
    size_t half = n / 2;
    
    for (size_t i = 0; i < half; ++i) {
        auto edge1 = boundary[i];
        auto edge2 = boundary[n - 1 - i];
        
        const auto& s1 = complex.get_simplex(edge1);
        const auto& s2 = complex.get_simplex(edge2);
        
        if (s1.dimension() == 1 && s2.dimension() == 1) {
            auto v1 = s1.vertices()[0];
            auto v2 = s2.vertices()[1];
            TopologyOperations::glue_vertices(complex, v1, v2);
            result.modified.push_back(edge1);
            result.modified.push_back(edge2);
        }
    }
    
    result.success = true;
    result.message = "Wrapped boundary to create non-orientable topology";
    
    return result;
}

// =============================================================================
// MorphPresets Implementation
// =============================================================================

std::vector<MorphRule> MorphPresets::default_rules() {
    std::vector<MorphRule> rules;
    
    // High absurdity -> split
    rules.emplace_back(
        "high_absurdity_split",
        MorphType::SPLIT,
        TriggerCondition::ABSURDITY_THRESHOLD,
        FuzzyInterval(0.7, 0.8, 0.9)
    );
    rules.back().probability = 0.8;
    rules.back().priority = 10;
    
    // Low absurdity -> merge
    rules.emplace_back(
        "low_absurdity_merge",
        MorphType::MERGE,
        TriggerCondition::LOW_ABSURDITY,
        FuzzyInterval(0.2, 0.3, 0.9)
    );
    rules.back().probability = 0.6;
    rules.back().priority = 5;
    
    // Very high absurdity -> delete
    rules.emplace_back(
        "very_high_absurdity_delete",
        MorphType::DELETE,
        TriggerCondition::ABSURDITY_THRESHOLD,
        FuzzyInterval(0.9, 0.95, 0.95)
    );
    rules.back().probability = 0.3;
    rules.back().priority = 20;
    
    // Neighbor difference -> expand
    rules.emplace_back(
        "neighbor_difference_expand",
        MorphType::EXPAND,
        TriggerCondition::NEIGHBOR_DIFFERENCE,
        FuzzyInterval(0.5, 0.6, 0.8)
    );
    rules.back().probability = 0.5;
    rules.back().priority = 3;
    
    return rules;
}

std::vector<MorphRule> MorphPresets::conservative_rules() {
    std::vector<MorphRule> rules;
    
    rules.emplace_back(
        "conservative_split",
        MorphType::SPLIT,
        TriggerCondition::ABSURDITY_THRESHOLD,
        FuzzyInterval(0.85, 0.9, 0.95)
    );
    rules.back().probability = 0.4;
    rules.back().priority = 5;
    
    rules.emplace_back(
        "conservative_merge",
        MorphType::MERGE,
        TriggerCondition::LOW_ABSURDITY,
        FuzzyInterval(0.1, 0.15, 0.95)
    );
    rules.back().probability = 0.3;
    rules.back().priority = 2;
    
    return rules;
}

std::vector<MorphRule> MorphPresets::aggressive_rules() {
    std::vector<MorphRule> rules;
    
    rules.emplace_back(
        "aggressive_split",
        MorphType::SPLIT,
        TriggerCondition::ABSURDITY_THRESHOLD,
        FuzzyInterval(0.4, 0.5, 0.8)
    );
    rules.back().probability = 0.9;
    rules.back().priority = 15;
    
    rules.emplace_back(
        "aggressive_delete",
        MorphType::DELETE,
        TriggerCondition::ABSURDITY_THRESHOLD,
        FuzzyInterval(0.7, 0.75, 0.8)
    );
    rules.back().probability = 0.6;
    rules.back().priority = 20;
    
    rules.emplace_back(
        "aggressive_create",
        MorphType::CREATE,
        TriggerCondition::LOW_ABSURDITY,
        FuzzyInterval(0.2, 0.25, 0.7)
    );
    rules.back().probability = 0.7;
    rules.back().priority = 10;
    
    return rules;
}

std::vector<MorphRule> MorphPresets::chaos_rules() {
    std::vector<MorphRule> rules;
    
    rules.emplace_back(
        "chaos_split",
        MorphType::SPLIT,
        TriggerCondition::ABSURDITY_THRESHOLD,
        FuzzyInterval(0.2, 0.3, 0.5)
    );
    rules.back().probability = 1.0;
    rules.back().priority = 10;
    
    rules.emplace_back(
        "chaos_delete",
        MorphType::DELETE,
        TriggerCondition::ABSURDITY_THRESHOLD,
        FuzzyInterval(0.4, 0.5, 0.6)
    );
    rules.back().probability = 0.8;
    rules.back().priority = 15;
    
    rules.emplace_back(
        "chaos_wrap",
        MorphType::WRAP,
        TriggerCondition::ABSURDITY_THRESHOLD,
        FuzzyInterval(0.6, 0.7, 0.7)
    );
    rules.back().probability = 0.5;
    rules.back().priority = 20;
    
    rules.emplace_back(
        "chaos_glue",
        MorphType::GLUE,
        TriggerCondition::ABSURDITY_THRESHOLD,
        FuzzyInterval(0.5, 0.6, 0.8)
    );
    rules.back().probability = 0.7;
    rules.back().priority = 5;
    
    return rules;
}

std::vector<MorphRule> MorphPresets::artistic_rules() {
    std::vector<MorphRule> rules;
    
    rules.emplace_back(
        "artistic_split",
        MorphType::SPLIT,
        TriggerCondition::ABSURDITY_THRESHOLD,
        FuzzyInterval(0.6, 0.7, 0.85)
    );
    rules.back().probability = 0.7;
    rules.back().priority = 10;
    
    rules.emplace_back(
        "artistic_refine",
        MorphType::REFINE,
        TriggerCondition::ABSURDITY_THRESHOLD,
        FuzzyInterval(0.5, 0.6, 0.8)
    );
    rules.back().probability = 0.6;
    rules.back().priority = 8;
    
    rules.emplace_back(
        "artistic_contract",
        MorphType::CONTRACT,
        TriggerCondition::LOW_ABSURDITY,
        FuzzyInterval(0.15, 0.2, 0.85)
    );
    rules.back().probability = 0.4;
    rules.back().priority = 5;
    
    return rules;
}

std::vector<MorphRule> MorphPresets::scientific_rules() {
    std::vector<MorphRule> rules;
    
    rules.emplace_back(
        "scientific_refine",
        MorphType::REFINE,
        TriggerCondition::ABSURDITY_THRESHOLD,
        FuzzyInterval(0.75, 0.8, 0.9)
    );
    rules.back().probability = 0.5;
    rules.back().priority = 10;
    rules.back().max_applications = 100;
    
    rules.emplace_back(
        "scientific_coarsen",
        MorphType::COARSEN,
        TriggerCondition::LOW_ABSURDITY,
        FuzzyInterval(0.1, 0.15, 0.9)
    );
    rules.back().probability = 0.3;
    rules.back().priority = 5;
    
    return rules;
}

// =============================================================================
// MorphQueue Implementation
// =============================================================================

void MorphQueue::enqueue(const QueuedMorph& morph) {
    morph.scheduled_time = morph.delay;
    queue_.push_back(morph);
}

std::vector<MorphResult> MorphQueue::execute_ready(
    double current_time,
    MorphContext& context
) {
    std::vector<MorphResult> results;
    
    TopologyMorph morph;
    
    auto it = queue_.begin();
    while (it != queue_.end()) {
        if (it->scheduled_time <= current_time) {
            auto result = morph.apply_morph(
                it->rule,
                it->target,
                context
            );
            results.push_back(result);
            
            it = queue_.erase(it);
        } else {
            ++it;
        }
    }
    
    return results;
}

size_t MorphQueue::cancel_for_simplex(SimplexID simplex_id) {
    size_t count = 0;
    
    auto it = queue_.begin();
    while (it != queue_.end()) {
        if (it->target == simplex_id) {
            it = queue_.erase(it);
            count++;
        } else {
            ++it;
        }
    }
    
    return count;
}

// =============================================================================
// MorphEventSystem Implementation
// =============================================================================

void MorphEventSystem::on_event(MorphEventType type, MorphEventCallback callback) {
    callbacks_[type].push_back(callback);
}

void MorphEventSystem::emit(const MorphEvent& event) {
    auto it = callbacks_.find(event.type);
    if (it != callbacks_.end()) {
        for (const auto& callback : it->second) {
            callback(event);
        }
    }
}

} // namespace cebu
