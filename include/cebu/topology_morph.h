#pragma once

#include "cebu/absurdity.h"
#include "cebu/simplicial_complex.h"
#include "cebu/simplex.h"
#include "cebu/label.h"
#include <vector>
#include <functional>
#include <optional>
#include <unordered_map>
#include <string>
#include <memory>

namespace cebu {

// =============================================================================
// Morph Rule - When and how to deform topology
// =============================================================================

/**
 * @brief Deformation type
 */
enum class MorphType {
    SPLIT,              ///< Split a simplex into smaller pieces
    MERGE,              ///< Merge adjacent simplices
    DELETE,             ///< Remove a simplex
    CREATE,             ///< Create new simplex
    EXPAND,             ///< Expand dimension (e.g., edge to face)
    CONTRACT,           ///< Contract dimension (e.g., face to edge)
    WRAP,               ///< Wrap around (create non-orientable)
    GLUE,               ///< Glue vertices together
    UNGLUE,             ///< Unglue vertices
    REFINE,             ///< Refine (add interior vertices)
    COARSEN             ///< Coarsen (remove interior vertices)
};

/**
 * @brief Trigger condition for morph operation
 */
enum class TriggerCondition {
    ABSURDITY_THRESHOLD,    ///< When absurdity exceeds threshold
    LOW_ABSURDITY,           ///< When absurdity is too low
    USER_INTERACTION,        ///< When user interacts
    TEMPORAL_EVENT,          ///< At specific time
    NEIGHBOR_DIFFERENCE,     ///< When neighbor differs significantly
    GLOBAL_THRESHOLD,        ///< Based on global absurdity
    CUSTOM                  ///< Custom predicate
};

/**
 * @brief Morph rule definition
 * 
 * A morph rule specifies:
 * 1. When to trigger (trigger condition)
 * 2. What operation to perform (morph type)
 * 3. How to perform it (morph function)
 */
struct MorphRule {
    std::string name;
    MorphType type;
    TriggerCondition condition;
    FuzzyInterval threshold;        // Threshold value
    double probability = 1.0;       // Probability of applying (0-1)
    size_t max_applications = 0;    // 0 = unlimited
    size_t application_count = 0;   // Applied count
    
    // Custom trigger function (if condition == CUSTOM)
    std::function<bool(const FuzzyInterval&, SimplexID)> custom_trigger;
    
    // Custom morph function (optional, provides specific behavior)
    std::function<bool(SimplicialComplex&, SimplexID)> custom_morph;
    
    // Priority (higher = checked first)
    int priority = 0;
    
    MorphRule(
        const std::string& n,
        MorphType t,
        TriggerCondition c,
        const FuzzyInterval& thresh
    ) : name(n), type(t), condition(c), threshold(thresh) {}
};

// =============================================================================
// Morph Operation - Single deformation
// =============================================================================

/**
 * @brief Result of a morph operation
 */
struct MorphResult {
    bool success;
    std::string message;
    std::vector<SimplexID> created;
    std::vector<SimplexID> modified;
    std::vector<SimplexID> removed;
    double absurdity_before;
    double absurdity_after;
    
    MorphResult()
        : success(false), absurdity_before(0.0), absurdity_after(0.0) {}
};

/**
 * @brief Morph operation context
 */
struct MorphContext {
    SimplicialComplex& complex;
    const AbsurdityField& absurdity_field;
    double time;
    double dt;
    FuzzyInterval global_absurdity;
    
    MorphContext(
        SimplicialComplex& c,
        const AbsurdityField& f,
        double t,
        double delta_t
    ) : complex(c), absurdity_field(f), time(t), dt(delta_t) {}
};

// =============================================================================
// Topology Morph - Dynamic deformation system
// =============================================================================

/**
 * @brief Dynamic topology deformation system
 * 
 * Deforms topology based on absurdity values. When absurdity exceeds
 * thresholds, morph rules are triggered to modify the simplicial complex.
 * 
 * This is the core mechanism by which absurdity drives structural change.
 */
class TopologyMorph {
public:
    /**
     * @brief Constructor
     */
    TopologyMorph();
    
    /**
     * @brief Register a morph rule
     * @param rule The rule to add
     */
    void add_rule(const MorphRule& rule);
    
    /**
     * @brief Remove a rule by name
     */
    bool remove_rule(const std::string& name);
    
    /**
     * @brief Get all rules
     */
    const std::vector<MorphRule>& get_rules() const { return rules_; }
    
    /**
     * @brief Apply morph rules to entire complex
     * @param context Morph context
     * @param max_operations Maximum operations to apply (0 = unlimited)
     * @return Vector of morph results
     */
    std::vector<MorphResult> apply_rules(
        MorphContext& context,
        size_t max_operations = 0
    );
    
    /**
     * @brief Check if any rules match for a simplex
     * @param simplex_id Simplex to check
     * @param context Morph context
     * @return First matching rule, or nullptr if none
     */
    const MorphRule* find_matching_rule(
        SimplexID simplex_id,
        const MorphContext& context
    ) const;
    
    /**
     * @brief Apply a single morph operation
     * @param rule Rule to apply
     * @param simplex_id Target simplex
     * @param context Morph context
     * @return Morph result
     */
    MorphResult apply_morph(
        const MorphRule& rule,
        SimplexID simplex_id,
        MorphContext& context
    );
    
    /**
     * @brief Get statistics
     */
    struct Stats {
        size_t total_operations = 0;
        size_t successful_operations = 0;
        size_t failed_operations = 0;
        std::unordered_map<MorphType, size_t> operation_counts;
        double total_absurdity_change = 0.0;
    };
    
    const Stats& get_stats() const { return stats_; }
    void reset_stats() { stats_ = Stats(); }
    
    /**
     * @brief Enable/disable morphing
     */
    void set_enabled(bool enabled) { enabled_ = enabled; }
    bool is_enabled() const { return enabled_; }
    
    // =============================================================================
    // Built-in morph operations
    // =============================================================================
    
    /**
     * @brief Split a simplex
     */
    static MorphResult split_simplex(
        SimplicialComplex& complex,
        SimplexID simplex_id,
        const AbsurdityField& field,
        size_t pieces = 2
    );
    
    /**
     * @brief Merge adjacent simplices
     */
    static MorphResult merge_simplices(
        SimplicialComplex& complex,
        SimplexID simplex_id,
        const AbsurdityField& field
    );
    
    /**
     * @brief Delete a simplex (cascade to dependents)
     */
    static MorphResult delete_simplex(
        SimplicialComplex& complex,
        SimplexID simplex_id,
        const AbsurdityField& field
    );
    
    /**
     * @brief Create new simplex in neighborhood
     */
    static MorphResult create_simplex(
        SimplicialComplex& complex,
        SimplexID simplex_id,
        const AbsurdityField& field,
        size_t dimension = 0
    );
    
    /**
     * @brief Expand dimension (e.g., edge to triangle)
     */
    static MorphResult expand_dimension(
        SimplicialComplex& complex,
        SimplexID simplex_id,
        const AbsurdityField& field
    );
    
    /**
     * @brief Contract dimension (e.g., triangle to edge)
     */
    static MorphResult contract_dimension(
        SimplicialComplex& complex,
        SimplexID simplex_id,
        const AbsurdityField& field
    );
    
    /**
     * @brief Glue vertices together
     */
    static MorphResult glue_vertices(
        SimplicialComplex& complex,
        SimplexID simplex_id,
        const AbsurdityField& field
    );
    
    /**
     * @brief Refine by adding interior vertex
     */
    static MorphResult refine_simplex(
        SimplicialComplex& complex,
        SimplexID simplex_id,
        const AbsurdityField& field
    );
    
    /**
     * @brief Coarsen by removing interior vertex
     */
    static MorphResult coarsen_simplex(
        SimplicialComplex& complex,
        SimplexID simplex_id,
        const AbsurdityField& field
    );
    
    /**
     * @brief Wrap to create non-orientable topology
     */
    static MorphResult wrap_boundary(
        SimplicialComplex& complex,
        SimplexID simplex_id,
        const AbsurdityField& field
    );
    
private:
    std::vector<MorphRule> rules_;
    Stats stats_;
    bool enabled_;
    
    // Helper: check if rule matches
    bool rule_matches(
        const MorphRule& rule,
        const FuzzyInterval& absurdity,
        const MorphContext& context,
        SimplexID simplex_id
    ) const;
    
    // Helper: update stats
    void update_stats(const MorphResult& result, MorphType type);
};

// =============================================================================
// Morph Preset - Predefined rule sets
// =============================================================================

/**
 * @brief Preset morph rule collections
 */
class MorphPresets {
public:
    /**
     * @brief Default rules for absurd-driven deformation
     */
    static std::vector<MorphRule> default_rules();
    
    /**
     * @brief Conservative rules (less aggressive)
     */
    static std::vector<MorphRule> conservative_rules();
    
    /**
     * @brief Aggressive rules (more deformation)
     */
    static std::vector<MorphRule> aggressive_rules();
    
    /**
     * @brief Chaos mode (extreme deformation)
     */
    static std::vector<MorphRule> chaos_rules();
    
    /**
     * @brief Artistic rules (controlled, aesthetic)
     */
    static std::vector<MorphRule> artistic_rules();
    
    /**
     * @brief Scientific rules (preserve structure)
     */
    static std::vector<MorphRule> scientific_rules();
};

// =============================================================================
// Morph Queue - Deferred morph operations
// =============================================================================

/**
 * @brief Queued morph operation
 */
struct QueuedMorph {
    MorphRule rule;
    SimplexID target;
    double delay;  // Time until execution
    double scheduled_time;
    
    QueuedMorph(const MorphRule& r, SimplexID t, double d = 0.0)
        : rule(r), target(t), delay(d), scheduled_time(0.0) {}
};

/**
 * @brief Queue for deferred morph operations
 */
class MorphQueue {
public:
    /**
     * @brief Add a morph to queue
     */
    void enqueue(const QueuedMorph& morph);
    
    /**
     * @brief Execute ready morphs
     * @param current_time Current time
     * @param context Morph context
     * @return Results of executed morphs
     */
    std::vector<MorphResult> execute_ready(
        double current_time,
        MorphContext& context
    );
    
    /**
     * @brief Cancel morphs for a simplex
     */
    size_t cancel_for_simplex(SimplexID simplex_id);
    
    /**
     * @brief Get queue size
     */
    size_t size() const { return queue_.size(); }
    
    /**
     * @brief Clear queue
     */
    void clear() { queue_.clear(); }
    
private:
    std::vector<QueuedMorph> queue_;
};

// =============================================================================
// Morph Event - Event system integration
// =============================================================================

/**
 * @brief Morph event types
 */
enum class MorphEventType {
    RULE_ADDED,
    RULE_REMOVED,
    MORPH_APPLIED,
    MORPH_FAILED,
    MORPH_QUEUED,
    MORPH_CANCELLED
};

/**
 * @brief Morph event data
 */
struct MorphEvent {
    MorphEventType type;
    std::string rule_name;
    SimplexID simplex_id;
    MorphResult result;
    double time;
    
    MorphEvent(MorphEventType t, const std::string& rn, SimplexID sid, double tm)
        : type(t), rule_name(rn), simplex_id(sid), time(tm) {}
};

/**
 * @brief Morph event callback type
 */
using MorphEventCallback = std::function<void(const MorphEvent&)>;

/**
 * @brief Event system for morph operations
 */
class MorphEventSystem {
public:
    /**
     * @brief Register event callback
     */
    void on_event(MorphEventType type, MorphEventCallback callback);
    
    /**
     * @brief Emit an event
     */
    void emit(const MorphEvent& event);
    
    /**
     * @brief Clear all callbacks
     */
    void clear() { callbacks_.clear(); }
    
private:
    std::unordered_map<MorphEventType, std::vector<MorphEventCallback>> callbacks_;
};

} // namespace cebu
