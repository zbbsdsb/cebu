#include "cebu/absurdity.h"
#include <cmath>
#include <algorithm>
#include <limits>
#include <stdexcept>

namespace cebu {

std::mt19937& Absurdity::get_rng() {
    static std::mt19937 rng(std::random_device{}());
    return rng;
}

Absurdity::Absurdity(double lower, double upper, double confidence)
    : lower_(std::clamp(lower, 0.0, 1.0)),
      upper_(std::clamp(upper, 0.0, 1.0)),
      confidence_(std::clamp(confidence, 0.0, 1.0)) {

    // Ensure lower <= upper
    if (lower_ > upper_) {
        std::swap(lower_, upper_);
    }
}

Absurdity Absurdity::update(const AbsurdityContext& ctx) const {
    // Evolution constants
    constexpr double kappa = 0.1;  // Coupling to target
    constexpr double eta = 0.05;    // Logical deviation weight
    constexpr double sigma = 0.02; // Noise strength
    constexpr double lambda = 0.01; // Width decay

    // Calculate target absurdity
    double target = 0.5 * ctx.surprisal + 0.5 * ctx.logical_deviation;

    // Modulate with user laughter (laughter directly increases absurdity)
    target = (1.0 - ctx.user_laughter) * target + ctx.user_laughter * 1.0;

    // Add noise (Gaussian)
    std::normal_distribution<double> dist(0.0, 1.0);
    double noise = sigma * midpoint() * dist(get_rng());

    // Update midpoint
    double new_mid = midpoint() + kappa * (target - midpoint()) * ctx.dt + noise;
    new_mid = std::clamp(new_mid, 0.0, 1.0);

    // Update width (decays over time, increases with uncertainty)
    double new_width = width() * std::exp(-lambda * ctx.dt) +
                       0.1 * (1.0 - confidence_);

    new_width = std::clamp(new_width, 0.0, 1.0);

    // Calculate new bounds
    double new_lower = std::clamp(new_mid - new_width / 2.0, 0.0, 1.0);
    double new_upper = std::clamp(new_mid + new_width / 2.0, 0.0, 1.0);

    // Update confidence (inversely related to width)
    double new_confidence = confidence_ * 0.99 + 0.01 * (1.0 - new_width);
    new_confidence = std::clamp(new_confidence, 0.0, 1.0);

    return Absurdity(new_lower, new_upper, new_confidence);
}

Absurdity Absurdity::operator+(const Absurdity& other) const {
    // Fuzzy addition: average the intervals
    double new_lower = (lower_ + other.lower_) / 2.0;
    double new_upper = (upper_ + other.upper_) / 2.0;
    double new_confidence = (confidence_ + other.confidence_) / 2.0;

    return Absurdity(new_lower, new_upper, new_confidence);
}

Absurdity Absurdity::operator*(double scalar) const {
    double new_lower = lower_ * scalar;
    double new_upper = upper_ * scalar;
    double new_confidence = confidence_;

    return Absurdity(new_lower, new_upper, new_confidence);
}

} // namespace cebu
