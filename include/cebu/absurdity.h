#pragma once

#include "cebu/label.h"
#include <string>
#include <random>

namespace cebu {

/**
 * @brief Context structure for absurdity updates
 *
 * Contains environmental factors that influence absurdity evolution.
 */
struct AbsurdityContext {
    double surprisal;              // Unexpectedness [0,1]
    double logical_deviation;      // Deviation from logic [0,1]
    double user_laughter;          // User laughter intensity [0,1]
    double narrative_tension;     // Narrative tension [0,1]
    double dt;                     // Time step

    AbsurdityContext()
        : surprisal(0.0), logical_deviation(0.0),
          user_laughter(0.0), narrative_tension(0.0), dt(1.0) {}

    AbsurdityContext(double s, double ld, double ul, double nt, double t)
        : surprisal(s), logical_deviation(ld),
          user_laughter(ul), narrative_tension(nt), dt(t) {}
};

/**
 * @brief Absurdity label representing interval-valued fuzzy numbers
 *
 * Absurdity is modeled as an interval fuzzy number:
 *   A = [lower, upper] x confidence
 *
 * Where:
 * - [lower, upper] is the possible range of absurdity [0,1]
 * - confidence is the reliability of the estimate [0,1]
 */
class Absurdity {
public:
    /**
     * @brief Constructor
     * @param lower Lower bound of absurdity range [0,1]
     * @param upper Upper bound of absurdity range [0,1]
     * @param confidence Confidence in the estimate [0,1]
     */
    Absurdity(double lower = 0.5, double upper = 0.5, double confidence = 1.0);

    /**
     * @brief Get lower bound
     */
    double lower() const { return lower_; }

    /**
     * @brief Get upper bound
     */
    double upper() const { return upper_; }

    /**
     * @brief Get confidence
     */
    double confidence() const { return confidence_; }

    /**
     * @brief Get midpoint of the interval
     */
    double midpoint() const { return (lower_ + upper_) / 2.0; }

    /**
     * @brief Get width of the interval
     */
    double width() const { return upper_ - lower_; }

    /**
     * @brief Update absurdity based on context
     * @param ctx Context containing environmental factors
     * @return New absurdity value
     */
    Absurdity update(const AbsurdityContext& ctx) const;

    /**
     * @brief Compare with threshold (checks if upper bound < threshold)
     */
    bool operator<(double threshold) const {
        return upper_ < threshold;
    }

    /**
     * @brief Compare with threshold (checks if lower bound > threshold)
     */
    bool operator>(double threshold) const {
        return lower_ > threshold;
    }

    /**
     * @brief Check if midpoint equals threshold (with tolerance)
     */
    bool operator==(double threshold) const {
        constexpr double epsilon = 1e-6;
        return std::abs(midpoint() - threshold) < epsilon;
    }

    /**
     * @brief Add two absurdities (fuzzy addition)
     */
    Absurdity operator+(const Absurdity& other) const;

    /**
     * @brief Scale absurdity
     */
    Absurdity operator*(double scalar) const;

    /**
     * @brief Get a single scalar value (using midpoint)
     */
    operator double() const { return midpoint(); }

private:
    double lower_;
    double upper_;
    double confidence_;

    static std::mt19937& get_rng();
};

/**
 * @brief Absurdity system with specialized queries
 */
class AbsurdityLabelSystem : public LabelSystem<Absurdity> {
public:
    AbsurdityLabelSystem() = default;
    ~AbsurdityLabelSystem() override = default;

    void set_label(SimplexID simplex_id, const Absurdity& label) override {
        labels_[simplex_id] = label;
    }

    std::optional<Absurdity> get_label(SimplexID simplex_id) const override {
        auto it = labels_.find(simplex_id);
        if (it != labels_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    std::vector<SimplexID> find_by_label(Predicate predicate) const override {
        std::vector<SimplexID> result;
        for (const auto& [id, label] : labels_) {
            if (predicate(label)) {
                result.push_back(id);
            }
        }
        return result;
    }

    /**
     * @brief Find simplices with absurdity midpoint in range
     */
    std::vector<SimplexID> find_in_range(double min, double max) const {
        std::vector<SimplexID> result;
        for (const auto& pair : labels_) {
            double mid = pair.second.midpoint();
            if (mid >= min && mid <= max) {
                result.push_back(pair.first);
            }
        }
        return result;
    }

    /**
     * @brief Find simplices with high absurdity (> threshold)
     */
    std::vector<SimplexID> find_high_absurdity(double threshold = 0.7) const {
        std::vector<SimplexID> result;
        for (const auto& pair : labels_) {
            if (pair.second > threshold) {
                result.push_back(pair.first);
            }
        }
        return result;
    }

    /**
     * @brief Find simplices with low absurdity (< threshold)
     */
    std::vector<SimplexID> find_low_absurdity(double threshold = 0.3) const {
        std::vector<SimplexID> result;
        for (const auto& pair : labels_) {
            if (pair.second < threshold) {
                result.push_back(pair.first);
            }
        }
        return result;
    }

    /**
     * @brief Find simplices with high uncertainty
     */
    std::vector<SimplexID> find_high_uncertainty(double threshold = 0.3) const {
        std::vector<SimplexID> result;
        for (const auto& pair : labels_) {
            if (pair.second.width() > threshold) {
                result.push_back(pair.first);
            }
        }
        return result;
    }

    bool remove_label(SimplexID simplex_id) override {
        return labels_.erase(simplex_id) > 0;
    }

    bool has_label(SimplexID simplex_id) const override {
        return labels_.find(simplex_id) != labels_.end();
    }

    size_t labeled_count() const override {
        return labels_.size();
    }

    void clear() override {
        labels_.clear();
    }

    /**
     * @brief Update all absurdities based on context
     */
    void update_all(const AbsurdityContext& ctx) {
        for (auto& pair : labels_) {
            pair.second = pair.second.update(ctx);
        }
    }

private:
    std::unordered_map<SimplexID, Absurdity> labels_;
};

/**
 * @brief Default context for absurdity updates
 */
inline AbsurdityContext default_context() {
    return AbsurdityContext(0.5, 0.0, 0.0, 0.5, 1.0);
}

} // namespace cebu
