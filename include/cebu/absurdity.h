#pragma once

#include <vector>
#include <string>
#include <optional>
#include <memory>
#include <random>
#include <functional>
#include <cmath>
#include <algorithm>
#include <sstream>

namespace cebu {

// =============================================================================
// Fuzzy Interval Number - Enhanced Absurdity Representation
// =============================================================================

/**
 * @brief Fuzzy interval number with confidence factor
 * 
 * Represents uncertainty as [lower, upper] × confidence
 * - lower: minimum plausible value
 * - upper: maximum plausible value  
 * - confidence: 0.0 to 1.0, how certain we are
 * 
 * Example: absurdity = [0.3, 0.7] × 0.8
 *   Means: we're 80% confident the absurdity is between 0.3 and 0.7
 */
class FuzzyInterval {
public:
    double lower;
    double upper;
    double confidence;

    FuzzyInterval(double l = 0.0, double u = 0.0, double c = 1.0)
        : lower(l), upper(u), confidence(std::clamp(c, 0.0, 1.0)) {
        ensure_valid();
    }

    void ensure_valid() {
        if (lower > upper) std::swap(lower, upper);
        lower = std::clamp(lower, 0.0, 1.0);
        upper = std::clamp(upper, 0.0, 1.0);
    }

    double midpoint() const { return (lower + upper) / 2.0; }
    double width() const { return upper - lower; }
    bool is_crisp() const { return width() < 1e-9; }
    bool is_certain() const { return confidence >= 0.9; }

    // Comparison operators with uncertainty awareness
    bool operator<(const FuzzyInterval& other) const;
    bool operator<=(const FuzzyInterval& other) const;
    bool operator>(const FuzzyInterval& other) const;
    bool operator>=(const FuzzyInterval& other) const;
    bool operator==(const FuzzyInterval& other) const;
    bool operator!=(const FuzzyInterval& other) const;

    // Arithmetic operations
    FuzzyInterval operator+(const FuzzyInterval& other) const;
    FuzzyInterval operator-(const FuzzyInterval& other) const;
    FuzzyInterval operator*(const FuzzyInterval& other) const;
    FuzzyInterval operator/(const FuzzyInterval& other) const;
    FuzzyInterval operator*(double scalar) const;

    // Uncertainty operations
    FuzzyInterval intersect(const FuzzyInterval& other) const;
    FuzzyInterval union_with(const FuzzyInterval& other) const;
    double overlap_degree(const FuzzyInterval& other) const;

    // String representation
    std::string to_string() const;
    std::string to_latex() const;

private:
    static std::mt19937& rng();
};

// =============================================================================
// Stochastic Evolution Equation
// =============================================================================

/**
 * @brief Noise generator for stochastic evolution
 */
class NoiseGenerator {
public:
    enum class Type {
        Gaussian,       // Normal distribution
        Uniform,        // Uniform distribution
        Poisson,        // Poisson distribution
        Levy            // Levy flight (heavy-tailed)
    };

    NoiseGenerator(Type type = Type::Gaussian, double mean = 0.0, double std = 0.1);

    double operator()() const;
    void seed(unsigned int s);

    // Access parameters
    Type get_type() const { return type_; }
    double get_mean() const { return mean_; }
    double get_std() const { return std_; }

    // Configure parameters
    void set_parameters(double mean, double std);

private:
    Type type_;
    double mean_;
    double std_;
    mutable std::mt19937 rng_;
};

/**
 * @brief Stochastic evolution parameters
 */
struct EvolutionParams {
    double decay_rate = 0.95;          // Decay factor per step
    double diffusion_rate = 0.05;       // Spatial diffusion
    double volatility = 0.1;           // Noise magnitude
    double coupling_strength = 0.3;    // Neighbor influence
    double threshold = 0.5;            // Activation threshold
    bool enable_levy_jumps = false;    // Enable rare large jumps
    double levy_exponent = 1.5;        // Levy distribution exponent
};

/**
 * @brief Stochastic evolution equation for absurdity
 * 
 * A_t+1 = decay * A_t + noise + coupling + diffusion
 * 
 * Where:
 * - A_t: current absurdity
 * - decay: exponential decay
 * - noise: stochastic perturbation
 * - coupling: influence from neighbors
 * - diffusion: spatial smoothing
 */
class StochasticEvolution {
public:
    StochasticEvolution(const EvolutionParams& params = EvolutionParams());
    explicit StochasticEvolution(NoiseGenerator noise);

    // Single step evolution
    FuzzyInterval step(const FuzzyInterval& current, double dt = 1.0);

    // Multi-step evolution
    std::vector<FuzzyInterval> evolve(
        const FuzzyInterval& initial,
        int steps,
        double dt = 1.0
    );

    // Neighborhood evolution (for simplices connected to each other)
    std::vector<FuzzyInterval> evolve_neighborhood(
        const std::vector<FuzzyInterval>& current,
        const std::vector<std::vector<size_t>>& adjacency,
        double dt = 1.0
    );

    // Access parameters
    EvolutionParams get_params() const { return params_; }
    void set_params(const EvolutionParams& params) { params_ = params; }

    // Noise generator access
    NoiseGenerator& noise() { return noise_; }
    const NoiseGenerator& noise() const { return noise_; }

    // Reset internal state
    void reset();

private:
    EvolutionParams params_;
    NoiseGenerator noise_;
    int step_count_;

    double generate_noise() const;
    double compute_coupling(
        const std::vector<FuzzyInterval>& values,
        const std::vector<size_t>& neighbors,
        size_t current_idx
    ) const;
    double compute_diffusion(
        const std::vector<FuzzyInterval>& values,
        const std::vector<size_t>& neighbors,
        size_t current_idx
    ) const;
};

// =============================================================================
// Multi-Source Fusion
// =============================================================================

/**
 * @brief Source of absurdity information
 */
struct AbsurditySource {
    std::string name;
    FuzzyInterval value;
    double reliability;  // 0.0 to 1.0
    double weight;       // Relative importance
    std::string metadata;

    bool is_valid() const {
        return reliability > 0.0 && value.confidence > 0.0;
    }
};

/**
 * @brief Fusion strategies for combining multiple sources
 */
class FusionStrategy {
public:
    enum class Method {
        WeightedAverage,     // Simple weighted average
        DempsterShafer,      // Dempster-Shafer evidence theory
        Bayesian,            // Bayesian updating
        FuzzyLogic,          // Fuzzy logic operations
        PossibilityTheory,   // Possibility theory
        Adaptive             // Automatic selection based on conflict
    };

    static Method default_method() { return Method::DempsterShafer; }

    /**
     * @brief Fuse multiple absurdity sources
     */
    static FuzzyInterval fuse(
        const std::vector<AbsurditySource>& sources,
        Method method = default_method()
    );

    /**
     * @brief Compute conflict between sources
     * 
     * Returns 0.0 if all sources agree, 1.0 if completely conflicting
     */
    static double compute_conflict(const std::vector<AbsurditySource>& sources);

    /**
     * @brief Select best fusion method based on data characteristics
     */
    static Method select_method(const std::vector<AbsurditySource>& sources);

private:
    // Specific fusion implementations
    static FuzzyInterval weighted_average(const std::vector<AbsurditySource>& sources);
    static FuzzyInterval dempster_shafer(const std::vector<AbsurditySource>& sources);
    static FuzzyInterval bayesian(const std::vector<AbsurditySource>& sources);
    static FuzzyInterval fuzzy_logic(const std::vector<AbsurditySource>& sources);
    static FuzzyInterval possibility_theory(const std::vector<AbsurditySource>& sources);

    // Helper functions
    static double normalize_weights(std::vector<AbsurditySource>& sources);
    static void discount_unreliable(std::vector<AbsurditySource>& sources, double threshold = 0.3);
};

// =============================================================================
// Uncertainty Comparison
// =============================================================================

/**
 * @brief Comparison results with uncertainty awareness
 */
struct ComparisonResult {
    enum class Relation {
        Less,          // Definitely less
        LessOrEqual,   // Possibly less or equal
        Equal,         // Possibly equal
        GreaterOrEqual,// Possibly greater or equal
        Greater,       // Definitely greater
        Incomparable   // Cannot determine
    };

    Relation relation;
    double confidence;  // 0.0 to 1.0
    std::string explanation;

    bool is_strict() const {
        return relation == Relation::Less || relation == Relation::Greater;
    }

    bool is_certain() const {
        return confidence >= 0.9;
    }
};

/**
 * @brief Uncertainty-aware comparison
 */
class UncertaintyComparison {
public:
    /**
     * @brief Compare two fuzzy intervals
     */
    static ComparisonResult compare(const FuzzyInterval& a, const FuzzyInterval& b);

    /**
     * @brief Check if a is definitely less than b
     */
    static bool definitely_less(const FuzzyInterval& a, const FuzzyInterval& b);

    /**
     * @brief Check if a is possibly less than b
     */
    static bool possibly_less(const FuzzyInterval& a, const FuzzyInterval& b);

    /**
     * @brief Check if a and b are possibly equal
     */
    static bool possibly_equal(const FuzzyInterval& a, const FuzzyInterval& b);

    /**
     * @brief Compute probability that a < b
     */
    static double probability_less(const FuzzyInterval& a, const FuzzyInterval& b);

    /**
     * @brief Sort with uncertainty awareness
     */
    static void sort_with_uncertainty(std::vector<FuzzyInterval>& values);

private:
    static ComparisonResult determine_relation(const FuzzyInterval& a, const FuzzyInterval& b);
    static double compute_overlap_confidence(const FuzzyInterval& a, const FuzzyInterval& b);
};

// =============================================================================
// Enhanced Absurdity Label
// =============================================================================

/**
 * @brief Enhanced label with fuzzy absurdity value
 * 
 * Extends the basic label with uncertainty-aware absurdity values
 */
class AbsurdityLabel {
public:
    AbsurdityLabel() = default;
    explicit AbsurdityLabel(const std::string& text);
    AbsurdityLabel(const std::string& text, const FuzzyInterval& absurdity);

    // Accessors
    const std::string& text() const { return text_; }
    const FuzzyInterval& absurdity() const { return absurdity_; }
    void set_absurdity(const FuzzyInterval& val) { absurdity_ = val; }

    // Computed properties
    double expected_absurdity() const { return absurdity_.midpoint(); }
    double uncertainty() const { return absurdity_.width(); }
    double confidence() const { return absurdity_.confidence; }

    // Comparison
    bool is_more_absurd(const AbsurdityLabel& other) const;
    bool is_less_absurd(const AbsurdityLabel& other) const;
    ComparisonResult compare_absurdity(const AbsurdityLabel& other) const;

    // String representation
    std::string to_string() const;

private:
    std::string text_;
    FuzzyInterval absurdity_;
};

// =============================================================================
// Absurdity Field - Spatial Distribution
// =============================================================================

/**
 * @brief Absurdity field over a simplicial complex
 * 
 * Manages absurdity values across all simplices with spatial correlation
 */
class AbsurdityField {
public:
    AbsurdityField() = default;

    // Set/get absurdity for a simplex
    void set(size_t simplex_id, const FuzzyInterval& absurdity);
    FuzzyInterval get(size_t simplex_id) const;

    // Batch operations
    void set_all(const FuzzyInterval& absurdity);
    void set_range(const std::vector<size_t>& ids, const FuzzyInterval& absurdity);

    // Spatial operations
    void diffuse(double rate, int iterations = 1);
    void evolve(const StochasticEvolution& evolution, double dt = 1.0);
    void evolve_with_coupling(
        const StochasticEvolution& evolution,
        const std::vector<std::vector<size_t>>& adjacency,
        double dt = 1.0
    );

    // Query operations
    std::vector<size_t> find_above_threshold(double threshold) const;
    std::vector<size_t> find_below_threshold(double threshold) const;
    std::vector<size_t> find_extremes(size_t n) const;

    // Statistics
    FuzzyInterval mean() const;
    FuzzyInterval variance() const;
    FuzzyInterval min() const;
    FuzzyInterval max() const;

    // Size
    size_t size() const { return values_.size(); }
    bool has(size_t simplex_id) const;

private:
    std::unordered_map<size_t, FuzzyInterval> values_;
};

// =============================================================================
// Utility Functions
// =============================================================================

namespace absurdity_utils {

    // Create crisp (certain) absurdity
    FuzzyInterval crisp(double value);

    // Create uncertain absurdity
    FuzzyInterval uncertain(double mean, double uncertainty, double confidence = 0.8);

    // Generate random absurdity
    FuzzyInterval random(double min_val = 0.0, double max_val = 1.0);

    // Interpolate between two absurdities
    FuzzyInterval interpolate(const FuzzyInterval& a, const FuzzyInterval& b, double t);

    // Clamp to valid range
    FuzzyInterval clamp(const FuzzyInterval& val, double min_val, double max_val);

    // Distance between two fuzzy intervals
    double distance(const FuzzyInterval& a, const FuzzyInterval& b);

    // Similarity between two fuzzy intervals
    double similarity(const FuzzyInterval& a, const FuzzyInterval& b);

} // namespace absurdity_utils

} // namespace cebu
