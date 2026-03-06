#include "cebu/absurdity.h"
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <numeric>

namespace cebu {

// =============================================================================
// FuzzyInterval Implementation
// =============================================================================

std::mt19937& FuzzyInterval::rng() {
    static std::mt19937 gen(std::random_device{}());
    return gen;
}

bool FuzzyInterval::operator<(const FuzzyInterval& other) const {
    // Conservative: definitely less if our max < their min
    if (upper < other.lower) return true;
    if (lower >= other.upper) return false;
    // Overlap - use midpoint with confidence weighting
    double w1 = confidence * width();
    double w2 = other.confidence * other.width();
    double effective_mean = (midpoint() + lower * w1 + upper * w1) / (1 + 2 * w1);
    double other_mean = (other.midpoint() + other.lower * w2 + other.upper * w2) / (1 + 2 * w2);
    return effective_mean < other_mean;
}

bool FuzzyInterval::operator<=(const FuzzyInterval& other) const {
    if (upper <= other.lower) return true;
    if (lower > other.upper) return false;
    // Overlap - use midpoint
    return midpoint() <= other.midpoint();
}

bool FuzzyInterval::operator>(const FuzzyInterval& other) const {
    return other < *this;
}

bool FuzzyInterval::operator>=(const FuzzyInterval& other) const {
    return other <= *this;
}

bool FuzzyInterval::operator==(const FuzzyInterval& other) const {
    // Crisp comparison
    if (is_crisp() && other.is_crisp()) {
        return std::abs(lower - other.lower) < 1e-9;
    }
    // Fuzzy comparison - check significant overlap
    return overlap_degree(other) > 0.5;
}

bool FuzzyInterval::operator!=(const FuzzyInterval& other) const {
    return !(*this == other);
}

FuzzyInterval FuzzyInterval::operator+(const FuzzyInterval& other) const {
    double new_lower = lower + other.lower;
    double new_upper = upper + other.upper;
    double new_confidence = std::min(confidence, other.confidence);
    return FuzzyInterval(new_lower, new_upper, new_confidence);
}

FuzzyInterval FuzzyInterval::operator-(const FuzzyInterval& other) const {
    double new_lower = lower - other.upper;
    double new_upper = upper - other.lower;
    double new_confidence = std::min(confidence, other.confidence);
    return FuzzyInterval(new_lower, new_upper, new_confidence);
}

FuzzyInterval FuzzyInterval::operator*(const FuzzyInterval& other) const {
    double candidates[4] = {
        lower * other.lower,
        lower * other.upper,
        upper * other.lower,
        upper * other.upper
    };
    double new_lower = *std::min_element(candidates, candidates + 4);
    double new_upper = *std::max_element(candidates, candidates + 4);
    double new_confidence = std::min(confidence, other.confidence);
    return FuzzyInterval(new_lower, new_upper, new_confidence);
}

FuzzyInterval FuzzyInterval::operator/(const FuzzyInterval& other) const {
    if (other.lower <= 0.0 && other.upper >= 0.0) {
        throw std::runtime_error("Division by interval containing zero");
    }
    double candidates[4] = {
        lower / other.lower,
        lower / other.upper,
        upper / other.lower,
        upper / other.upper
    };
    double new_lower = *std::min_element(candidates, candidates + 4);
    double new_upper = *std::max_element(candidates, candidates + 4);
    double new_confidence = std::min(confidence, other.confidence);
    return FuzzyInterval(new_lower, new_upper, new_confidence);
}

FuzzyInterval FuzzyInterval::operator*(double scalar) const {
    if (scalar >= 0) {
        return FuzzyInterval(lower * scalar, upper * scalar, confidence);
    } else {
        return FuzzyInterval(upper * scalar, lower * scalar, confidence);
    }
}

FuzzyInterval FuzzyInterval::intersect(const FuzzyInterval& other) const {
    double new_lower = std::max(lower, other.lower);
    double new_upper = std::min(upper, other.upper);
    if (new_lower > new_upper) {
        return FuzzyInterval(0, 0, 0);  // Empty intersection
    }
    double new_confidence = std::min(confidence, other.confidence);
    return FuzzyInterval(new_lower, new_upper, new_confidence);
}

FuzzyInterval FuzzyInterval::union_with(const FuzzyInterval& other) const {
    double new_lower = std::min(lower, other.lower);
    double new_upper = std::max(upper, other.upper);
    double new_confidence = std::max(confidence, other.confidence);
    return FuzzyInterval(new_lower, new_upper, new_confidence);
}

double FuzzyInterval::overlap_degree(const FuzzyInterval& other) const {
    double overlap_low = std::max(lower, other.lower);
    double overlap_high = std::min(upper, other.upper);
    
    if (overlap_low >= overlap_high) return 0.0;
    
    double overlap_length = overlap_high - overlap_low;
    double union_length = std::max(upper, other.upper) - std::min(lower, other.lower);
    
    return overlap_length / union_length;
}

std::string FuzzyInterval::to_string() const {
    std::ostringstream oss;
    oss << "[" << std::fixed << std::setprecision(3)
        << lower << ", " << upper << "] × " << std::setprecision(2)
        << confidence;
    return oss.str();
}

std::string FuzzyInterval::to_latex() const {
    std::ostringstream oss;
    oss << "[" << std::fixed << std::setprecision(3)
        << lower << ", " << upper << "] \\times " << std::setprecision(2)
        << confidence;
    return oss.str();
}

// =============================================================================
// NoiseGenerator Implementation
// =============================================================================

NoiseGenerator::NoiseGenerator(Type type, double mean, double std)
    : type_(type), mean_(mean), std_(std), rng_(std::random_device{}()) {
}

double NoiseGenerator::operator()() const {
    std::uniform_real_distribution<double> uniform(0.0, 1.0);
    
    switch (type_) {
        case Type::Gaussian: {
            std::normal_distribution<double> normal(mean_, std_);
            return normal(rng_);
        }
        case Type::Uniform: {
            std::uniform_real_distribution<double> dist(mean_ - std_, mean_ + std_);
            return dist(rng_);
        }
        case Type::Poisson: {
            std::poisson_distribution<int> poisson(mean_);
            return static_cast<double>(poisson(rng_)) - mean_;
        }
        case Type::Levy: {
            // Box-Muller transform for normal, then power for Levy
            std::normal_distribution<double> normal(0.0, 1.0);
            double u = normal(rng_);
            double v = normal(rng_);
            double t = u / std::pow(std::abs(v), 1.5);
            return mean_ + std_ * t;
        }
        default:
            return 0.0;
    }
}

void NoiseGenerator::seed(unsigned int s) {
    rng_.seed(s);
}

void NoiseGenerator::set_parameters(double mean, double std) {
    mean_ = mean;
    std_ = std;
}

// =============================================================================
// StochasticEvolution Implementation
// =============================================================================

StochasticEvolution::StochasticEvolution(const EvolutionParams& params)
    : params_(params), noise_(NoiseGenerator::Type::Gaussian, 0.0, params.volatility), step_count_(0) {
}

StochasticEvolution::StochasticEvolution(NoiseGenerator noise)
    : params_(), noise_(std::move(noise)), step_count_(0) {
    params_.volatility = noise_.get_std();
}

double StochasticEvolution::generate_noise() const {
    double n = noise_();
    if (params_.enable_levy_jumps && std::uniform_real_distribution<double>(0.0, 1.0)(noise_.rng_) < 0.05) {
        // 5% chance of large Levy jump
        n *= std::pow(std::abs(n), 1.0 - params_.levy_exponent);
    }
    return n;
}

double StochasticEvolution::compute_coupling(
    const std::vector<FuzzyInterval>& values,
    const std::vector<size_t>& neighbors,
    size_t current_idx
) const {
    if (neighbors.empty()) return 0.0;
    
    double sum = 0.0;
    for (size_t n : neighbors) {
        sum += values[n].midpoint() - values[current_idx].midpoint();
    }
    return params_.coupling_strength * sum / neighbors.size();
}

double StochasticEvolution::compute_diffusion(
    const std::vector<FuzzyInterval>& values,
    const std::vector<size_t>& neighbors,
    size_t current_idx
) const {
    if (neighbors.empty()) return 0.0;
    
    double center = values[current_idx].midpoint();
    double sum = 0.0;
    for (size_t n : neighbors) {
        sum += values[n].midpoint() - center;
    }
    return params_.diffusion_rate * sum / neighbors.size();
}

FuzzyInterval StochasticEvolution::step(const FuzzyInterval& current, double dt) {
    // Apply decay
    double decay = std::pow(params_.decay_rate, dt);
    FuzzyInterval decayed = current * decay;
    
    // Add noise
    double noise_val = generate_noise();
    double new_lower = decayed.lower + noise_val;
    double new_upper = decayed.upper + noise_val;
    
    // Adjust confidence based on noise
    double confidence_loss = params_.volatility * dt * 0.1;
    double new_confidence = std::max(0.0, decayed.confidence - confidence_loss);
    
    // Clamp to valid range
    FuzzyInterval result(new_lower, new_upper, new_confidence);
    result.ensure_valid();
    
    step_count_++;
    return result;
}

std::vector<FuzzyInterval> StochasticEvolution::evolve(
    const FuzzyInterval& initial,
    int steps,
    double dt
) {
    std::vector<FuzzyInterval> trajectory(steps + 1);
    trajectory[0] = initial;
    
    FuzzyInterval current = initial;
    for (int i = 0; i < steps; ++i) {
        current = step(current, dt);
        trajectory[i + 1] = current;
    }
    
    return trajectory;
}

std::vector<FuzzyInterval> StochasticEvolution::evolve_neighborhood(
    const std::vector<FuzzyInterval>& current,
    const std::vector<std::vector<size_t>>& adjacency,
    double dt
) {
    if (current.size() != adjacency.size()) {
        throw std::runtime_error("Size mismatch between values and adjacency");
    }
    
    std::vector<FuzzyInterval> next = current;
    
    for (size_t i = 0; i < current.size(); ++i) {
        // Apply individual evolution
        next[i] = step(current[i], dt);
        
        // Add coupling from neighbors
        double coupling = compute_coupling(current, adjacency[i], i);
        next[i].lower += coupling;
        next[i].upper += coupling;
        
        // Add diffusion from neighbors
        double diffusion = compute_diffusion(current, adjacency[i], i);
        next[i].lower += diffusion;
        next[i].upper += diffusion;
        
        // Ensure validity
        next[i].ensure_valid();
    }
    
    return next;
}

void StochasticEvolution::reset() {
    step_count_ = 0;
}

// =============================================================================
// FusionStrategy Implementation
// =============================================================================

FuzzyInterval FusionStrategy::fuse(
    const std::vector<AbsurditySource>& sources,
    Method method
) {
    if (sources.empty()) {
        throw std::runtime_error("Cannot fuse empty source list");
    }
    
    if (sources.size() == 1) {
        return sources[0].value;
    }
    
    // Create working copy
    std::vector<AbsurditySource> working = sources;
    
    // Normalize weights and discount unreliable sources
    normalize_weights(working);
    discount_unreliable(working);
    
    // Apply fusion method
    switch (method) {
        case Method::WeightedAverage:
            return weighted_average(working);
        case Method::DempsterShafer:
            return dempster_shafer(working);
        case Method::Bayesian:
            return bayesian(working);
        case Method::FuzzyLogic:
            return fuzzy_logic(working);
        case Method::PossibilityTheory:
            return possibility_theory(working);
        case Method::Adaptive:
            return fuse(working, select_method(working));
        default:
            return weighted_average(working);
    }
}

double FusionStrategy::compute_conflict(const std::vector<AbsurditySource>& sources) {
    if (sources.size() < 2) return 0.0;
    
    double total_conflict = 0.0;
    int comparisons = 0;
    
    for (size_t i = 0; i < sources.size(); ++i) {
        for (size_t j = i + 1; j < sources.size(); ++j) {
            double overlap = sources[i].value.overlap_degree(sources[j].value);
            total_conflict += (1.0 - overlap);
            comparisons++;
        }
    }
    
    return comparisons > 0 ? total_conflict / comparisons : 0.0;
}

FusionStrategy::Method FusionStrategy::select_method(const std::vector<AbsurditySource>& sources) {
    double conflict = compute_conflict(sources);
    
    if (conflict < 0.2) {
        return Method::WeightedAverage;  // Low conflict, simple average
    } else if (conflict < 0.5) {
        return Method::FuzzyLogic;  // Moderate conflict, fuzzy operations
    } else if (conflict < 0.7) {
        return Method::DempsterShafer;  // High conflict, evidence theory
    } else {
        return Method::PossibilityTheory;  // Very high conflict, possibility theory
    }
}

double FusionStrategy::normalize_weights(std::vector<AbsurditySource>& sources) {
    double total_weight = 0.0;
    for (const auto& src : sources) {
        total_weight += src.weight;
    }
    
    if (total_weight > 0.0) {
        for (auto& src : sources) {
            src.weight /= total_weight;
        }
    }
    
    return total_weight;
}

void FusionStrategy::discount_unreliable(std::vector<AbsurditySource>& sources, double threshold) {
    for (auto& src : sources) {
        if (src.reliability < threshold) {
            src.weight *= src.reliability;  // Downweight unreliable sources
        }
    }
}

FuzzyInterval FusionStrategy::weighted_average(const std::vector<AbsurditySource>& sources) {
    double sum_lower = 0.0;
    double sum_upper = 0.0;
    double sum_confidence = 0.0;
    
    for (const auto& src : sources) {
        sum_lower += src.weight * src.value.lower;
        sum_upper += src.weight * src.value.upper;
        sum_confidence += src.weight * src.value.confidence;
    }
    
    return FuzzyInterval(sum_lower, sum_upper, sum_confidence);
}

FuzzyInterval FusionStrategy::dempster_shafer(const std::vector<AbsurditySource>& sources) {
    // Simplified Dempster-Shafer combination
    double combined_lower = 1.0;
    double combined_upper = 0.0;
    double combined_confidence = 0.0;
    
    for (const auto& src : sources) {
        double mass = src.weight * src.reliability;
        combined_lower *= std::pow(src.value.lower, mass);
        combined_upper *= std::pow(src.value.upper, mass);
        combined_confidence += mass * src.value.confidence;
    }
    
    combined_confidence = std::min(1.0, combined_confidence);
    return FuzzyInterval(combined_lower, combined_upper, combined_confidence);
}

FuzzyInterval FusionStrategy::bayesian(const std::vector<AbsurditySource>& sources) {
    // Bayesian updating
    FuzzyInterval posterior(0.5, 0.5, 0.5);  // Uniform prior
    
    for (const auto& src : sources) {
        double evidence = src.weight * src.reliability;
        posterior.lower = posterior.lower * (1.0 - evidence) + src.value.lower * evidence;
        posterior.upper = posterior.upper * (1.0 - evidence) + src.value.upper * evidence;
        posterior.confidence = posterior.confidence * (1.0 - evidence) + src.value.confidence * evidence;
    }
    
    return posterior;
}

FuzzyInterval FusionStrategy::fuzzy_logic(const std::vector<AbsurditySource>& sources) {
    // Fuzzy intersection and union
    FuzzyInterval result(1.0, 1.0, 1.0);
    
    for (const auto& src : sources) {
        result.lower = std::max(result.lower, src.value.lower);
        result.upper = std::min(result.upper, src.value.upper);
        result.confidence = std::min(result.confidence, src.value.confidence * src.reliability);
    }
    
    return result;
}

FuzzyInterval FusionStrategy::possibility_theory(const std::vector<AbsurditySource>& sources) {
    // Possibility and necessity measures
    double max_lower = 0.0;
    double min_upper = 1.0;
    double avg_confidence = 0.0;
    
    for (const auto& src : sources) {
        max_lower = std::max(max_lower, src.value.lower);
        min_upper = std::min(min_upper, src.value.upper);
        avg_confidence += src.value.confidence;
    }
    
    avg_confidence /= sources.size();
    return FuzzyInterval(max_lower, min_upper, avg_confidence);
}

// =============================================================================
// UncertaintyComparison Implementation
// =============================================================================

ComparisonResult UncertaintyComparison::compare(const FuzzyInterval& a, const FuzzyInterval& b) {
    ComparisonResult result;
    result.relation = determine_relation(a, b);
    result.confidence = compute_overlap_confidence(a, b);
    
    switch (result.relation) {
        case ComparisonResult::Relation::Less:
            result.explanation = "a is definitely less than b";
            break;
        case ComparisonResult::Relation::LessOrEqual:
            result.explanation = "a is possibly less than or equal to b";
            break;
        case ComparisonResult::Relation::Equal:
            result.explanation = "a and b are possibly equal";
            break;
        case ComparisonResult::Relation::GreaterOrEqual:
            result.explanation = "a is possibly greater than or equal to b";
            break;
        case ComparisonResult::Relation::Greater:
            result.explanation = "a is definitely greater than b";
            break;
        case ComparisonResult::Relation::Incomparable:
            result.explanation = "a and b cannot be compared due to high uncertainty";
            break;
    }
    
    return result;
}

ComparisonResult::Relation UncertaintyComparison::determine_relation(
    const FuzzyInterval& a,
    const FuzzyInterval& b
) {
    if (a.upper < b.lower) {
        return ComparisonResult::Relation::Less;
    } else if (a.lower > b.upper) {
        return ComparisonResult::Relation::Greater;
    } else if (a.midpoint() < b.midpoint()) {
        if (a.upper < b.midpoint()) {
            return ComparisonResult::Relation::LessOrEqual;
        }
        return ComparisonResult::Relation::LessOrEqual;
    } else if (a.midpoint() > b.midpoint()) {
        if (a.lower > b.midpoint()) {
            return ComparisonResult::Relation::GreaterOrEqual;
        }
        return ComparisonResult::Relation::GreaterOrEqual;
    } else {
        return ComparisonResult::Relation::Equal;
    }
}

double UncertaintyComparison::compute_overlap_confidence(const FuzzyInterval& a, const FuzzyInterval& b) {
    if (a.upper < b.lower || a.lower > b.upper) {
        return 1.0;  // No overlap, high confidence
    }
    
    double overlap = a.overlap_degree(b);
    return 1.0 - overlap * 0.5;  // Higher overlap = lower confidence
}

bool UncertaintyComparison::definitely_less(const FuzzyInterval& a, const FuzzyInterval& b) {
    return a.upper < b.lower;
}

bool UncertaintyComparison::possibly_less(const FuzzyInterval& a, const FuzzyInterval& b) {
    return a.midpoint() < b.midpoint() || a.upper < b.midpoint();
}

bool UncertaintyComparison::possibly_equal(const FuzzyInterval& a, const FuzzyInterval& b) {
    return a.overlap_degree(b) > 0.3;
}

double UncertaintyComparison::probability_less(const FuzzyInterval& a, const FuzzyInterval& b) {
    if (a.upper <= b.lower) return 1.0;
    if (a.lower >= b.upper) return 0.0;
    
    // Estimate probability using overlap
    double overlap = a.overlap_degree(b);
    double a_center = a.midpoint();
    double b_center = b.midpoint();
    
    if (a_center < b_center) {
        return 0.5 + (0.5 - overlap) * 0.5;
    } else {
        return 0.5 - (0.5 - overlap) * 0.5;
    }
}

void UncertaintyComparison::sort_with_uncertainty(std::vector<FuzzyInterval>& values) {
    std::sort(values.begin(), values.end(),
        [](const FuzzyInterval& a, const FuzzyInterval& b) {
            return a < b;
        });
}

// =============================================================================
// AbsurdityLabel Implementation
// =============================================================================

AbsurdityLabel::AbsurdityLabel(const std::string& text)
    : text_(text), absurdity_(0.0, 0.0, 1.0) {
}

AbsurdityLabel::AbsurdityLabel(const std::string& text, const FuzzyInterval& absurdity)
    : text_(text), absurdity_(absurdity) {
}

bool AbsurdityLabel::is_more_absurd(const AbsurdityLabel& other) const {
    return absurdity_ > other.absurdity_;
}

bool AbsurdityLabel::is_less_absurd(const AbsurdityLabel& other) const {
    return absurdity_ < other.absurdity_;
}

ComparisonResult AbsurdityLabel::compare_absurdity(const AbsurdityLabel& other) const {
    return UncertaintyComparison::compare(absurdity_, other.absurdity_);
}

std::string AbsurdityLabel::to_string() const {
    std::ostringstream oss;
    oss << "\"" << text_ << "\" (" << absurdity_.to_string() << ")";
    return oss.str();
}

// =============================================================================
// AbsurdityField Implementation
// =============================================================================

void AbsurdityField::set(size_t simplex_id, const FuzzyInterval& absurdity) {
    values_[simplex_id] = absurdity;
}

FuzzyInterval AbsurdityField::get(size_t simplex_id) const {
    auto it = values_.find(simplex_id);
    if (it != values_.end()) {
        return it->second;
    }
    return FuzzyInterval(0.0, 0.0, 0.0);  // Default value
}

void AbsurdityField::set_all(const FuzzyInterval& absurdity) {
    for (auto& pair : values_) {
        pair.second = absurdity;
    }
}

void AbsurdityField::set_range(const std::vector<size_t>& ids, const FuzzyInterval& absurdity) {
    for (size_t id : ids) {
        values_[id] = absurdity;
    }
}

void AbsurdityField::diffuse(double rate, int iterations) {
    for (int iter = 0; iter < iterations; ++iter) {
        std::unordered_map<size_t, FuzzyInterval> new_values = values_;
        
        for (const auto& pair : values_) {
            size_t id = pair.first;
            FuzzyInterval current = pair.second;
            
            // Simple diffusion toward mean
            double sum_lower = current.lower;
            double sum_upper = current.upper;
            int count = 1;
            
            for (const auto& other : values_) {
                if (other.first != id) {
                    sum_lower += other.second.lower;
                    sum_upper += other.second.upper;
                    count++;
                }
            }
            
            double mean_lower = sum_lower / count;
            double mean_upper = sum_upper / count;
            
            new_values[id].lower += rate * (mean_lower - current.lower);
            new_values[id].upper += rate * (mean_upper - current.upper);
            new_values[id].ensure_valid();
        }
        
        values_ = new_values;
    }
}

void AbsurdityField::evolve(const StochasticEvolution& evolution, double dt) {
    for (auto& pair : values_) {
        pair.second = evolution.step(pair.second, dt);
    }
}

void AbsurdityField::evolve_with_coupling(
    const StochasticEvolution& evolution,
    const std::vector<std::vector<size_t>>& adjacency,
    double dt
) {
    // Build current values vector
    std::vector<FuzzyInterval> current_values(adjacency.size());
    for (size_t i = 0; i < adjacency.size(); ++i) {
        current_values[i] = get(i);
    }
    
    // Evolve
    auto next_values = evolution.evolve_neighborhood(current_values, adjacency, dt);
    
    // Update
    for (size_t i = 0; i < next_values.size(); ++i) {
        set(i, next_values[i]);
    }
}

std::vector<size_t> AbsurdityField::find_above_threshold(double threshold) const {
    std::vector<size_t> result;
    for (const auto& pair : values_) {
        if (pair.second.midpoint() > threshold) {
            result.push_back(pair.first);
        }
    }
    return result;
}

std::vector<size_t> AbsurdityField::find_below_threshold(double threshold) const {
    std::vector<size_t> result;
    for (const auto& pair : values_) {
        if (pair.second.midpoint() < threshold) {
            result.push_back(pair.first);
        }
    }
    return result;
}

std::vector<size_t> AbsurdityField::find_extremes(size_t n) const {
    std::vector<std::pair<double, size_t>> sorted;
    for (const auto& pair : values_) {
        sorted.emplace_back(pair.second.midpoint(), pair.first);
    }
    
    std::partial_sort(sorted.begin(), sorted.begin() + std::min(n, sorted.size()),
        sorted.end(),
        [](const auto& a, const auto& b) { return a.first > b.first; });
    
    std::vector<size_t> result;
    for (size_t i = 0; i < std::min(n, sorted.size()); ++i) {
        result.push_back(sorted[i].second);
    }
    return result;
}

FuzzyInterval AbsurdityField::mean() const {
    if (values_.empty()) return FuzzyInterval(0.0, 0.0, 0.0);
    
    double sum_lower = 0.0;
    double sum_upper = 0.0;
    double sum_confidence = 0.0;
    
    for (const auto& pair : values_) {
        sum_lower += pair.second.lower;
        sum_upper += pair.second.upper;
        sum_confidence += pair.second.confidence;
    }
    
    size_t n = values_.size();
    return FuzzyInterval(sum_lower / n, sum_upper / n, sum_confidence / n);
}

FuzzyInterval AbsurdityField::variance() const {
    if (values_.empty()) return FuzzyInterval(0.0, 0.0, 0.0);
    
    FuzzyInterval m = mean();
    double sum_sq_lower = 0.0;
    double sum_sq_upper = 0.0;
    
    for (const auto& pair : values_) {
        sum_sq_lower += std::pow(pair.second.lower - m.lower, 2);
        sum_sq_upper += std::pow(pair.second.upper - m.upper, 2);
    }
    
    size_t n = values_.size();
    return FuzzyInterval(sum_sq_lower / n, sum_sq_upper / n, m.confidence);
}

FuzzyInterval AbsurdityField::min() const {
    if (values_.empty()) return FuzzyInterval(0.0, 0.0, 0.0);
    
    auto it = std::min_element(values_.begin(), values_.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; });
    
    return it->second;
}

FuzzyInterval AbsurdityField::max() const {
    if (values_.empty()) return FuzzyInterval(0.0, 0.0, 0.0);
    
    auto it = std::max_element(values_.begin(), values_.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; });
    
    return it->second;
}

bool AbsurdityField::has(size_t simplex_id) const {
    return values_.find(simplex_id) != values_.end();
}

// =============================================================================
// Utility Functions
// =============================================================================

namespace absurdity_utils {

FuzzyInterval crisp(double value) {
    return FuzzyInterval(value, value, 1.0);
}

FuzzyInterval uncertain(double mean, double uncertainty, double confidence) {
    return FuzzyInterval(mean - uncertainty, mean + uncertainty, confidence);
}

FuzzyInterval random(double min_val, double max_val) {
    static std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<double> dist(min_val, max_val);
    
    double lower = dist(gen);
    double upper = dist(gen);
    double confidence = std::uniform_real_distribution<double>(0.5, 1.0)(gen);
    
    return FuzzyInterval(lower, upper, confidence);
}

FuzzyInterval interpolate(const FuzzyInterval& a, const FuzzyInterval& b, double t) {
    t = std::clamp(t, 0.0, 1.0);
    double lower = a.lower + t * (b.lower - a.lower);
    double upper = a.upper + t * (b.upper - a.upper);
    double confidence = a.confidence + t * (b.confidence - a.confidence);
    return FuzzyInterval(lower, upper, confidence);
}

FuzzyInterval clamp(const FuzzyInterval& val, double min_val, double max_val) {
    return FuzzyInterval(
        std::clamp(val.lower, min_val, max_val),
        std::clamp(val.upper, min_val, max_val),
        val.confidence
    );
}

double distance(const FuzzyInterval& a, const FuzzyInterval& b) {
    double center_diff = std::abs(a.midpoint() - b.midpoint());
    double width_diff = std::abs(a.width() - b.width());
    return center_diff + 0.5 * width_diff;
}

double similarity(const FuzzyInterval& a, const FuzzyInterval& b) {
    double dist = distance(a, b);
    return std::exp(-dist);
}

} // namespace absurdity_utils

} // namespace cebu
