#pragma once

#include <string>
#include <vector>
#include <functional>
#include <optional>

namespace cebu {

/**
 * @brief Empty label type for simplices without labels
 */
struct NoLabel {};

/**
 * @brief Basic label interface
 *
 * Labels can be attached to simplices to store semantic information
 * such as absurdity, narrative tension, or any custom data.
 */
template<typename T>
class LabelSystem {
public:
    using LabelType = T;
    using Predicate = std::function<bool(const T&)>;

    LabelSystem() = default;
    virtual ~LabelSystem() = default;

    /**
     * @brief Set a label for a simplex
     * @param simplex_id ID of the simplex
     * @param label Label value
     */
    virtual void set_label(SimplexID simplex_id, const T& label) = 0;

    /**
     * @brief Get the label for a simplex
     * @param simplex_id ID of the simplex
     * @return Optional containing the label, or empty if not set
     */
    virtual std::optional<T> get_label(SimplexID simplex_id) const = 0;

    /**
     * @brief Find all simplices with labels matching a predicate
     * @param predicate Function that returns true for matching labels
     * @return List of simplex IDs
     */
    virtual std::vector<SimplexID> find_by_label(Predicate predicate) const = 0;

    /**
     * @brief Remove a label from a simplex
     * @param simplex_id ID of the simplex
     * @return true if label was removed, false if it didn't exist
     */
    virtual bool remove_label(SimplexID simplex_id) = 0;

    /**
     * @brief Check if a simplex has a label
     * @param simplex_id ID of the simplex
     */
    virtual bool has_label(SimplexID simplex_id) const = 0;

    /**
     * @brief Get the number of labeled simplices
     */
    virtual size_t labeled_count() const = 0;

    /**
     * @brief Clear all labels
     */
    virtual void clear() = 0;
};

/**
 * @brief Default label system using unordered_map for storage
 */
template<typename T>
class DefaultLabelSystem : public LabelSystem<T> {
public:
    using LabelType = T;
    using Predicate = std::function<bool(const T&)>;

    DefaultLabelSystem() = default;
    ~DefaultLabelSystem() override = default;

    void set_label(SimplexID simplex_id, const T& label) override {
        labels_[simplex_id] = label;
    }

    std::optional<T> get_label(SimplexID simplex_id) const override {
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
     * @brief Get all labels
     */
    const std::unordered_map<SimplexID, T>& get_all_labels() const {
        return labels_;
    }

private:
    std::unordered_map<SimplexID, T> labels_;
};

/**
 * @brief Label system optimized for numeric labels with range queries
 */
class NumericLabelSystem : public LabelSystem<double> {
public:
    NumericLabelSystem() = default;
    ~NumericLabelSystem() override = default;

    void set_label(SimplexID simplex_id, double label) override {
        labels_[simplex_id] = label;
    }

    std::optional<double> get_label(SimplexID simplex_id) const override {
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
     * @brief Find simplices with labels in a range [min, max]
     * @param min Minimum value (inclusive)
     * @param max Maximum value (inclusive)
     */
    std::vector<SimplexID> find_in_range(double min, double max) const {
        std::vector<SimplexID> result;
        for (const auto& [id, label] : labels_) {
            if (label >= min && label <= max) {
                result.push_back(id);
            }
        }
        return result;
    }

    /**
     * @brief Find simplices with labels greater than a threshold
     */
    std::vector<SimplexID> find_greater_than(double threshold) const {
        std::vector<SimplexID> result;
        for (const auto& [id, label] : labels_) {
            if (label > threshold) {
                result.push_back(id);
            }
        }
        return result;
    }

    /**
     * @brief Find simplices with labels less than a threshold
     */
    std::vector<SimplexID> find_less_than(double threshold) const {
        std::vector<SimplexID> result;
        for (const auto& [id, label] : labels_) {
            if (label < threshold) {
                result.push_back(id);
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
     * @brief Get the minimum label value
     */
    std::optional<double> min_label() const {
        if (labels_.empty()) return std::nullopt;
        auto it = std::min_element(labels_.begin(), labels_.end(),
                                   [](const auto& a, const auto& b) {
                                       return a.second < b.second;
                                   });
        return it->second;
    }

    /**
     * @brief Get the maximum label value
     */
    std::optional<double> max_label() const {
        if (labels_.empty()) return std::nullopt;
        auto it = std::max_element(labels_.begin(), labels_.end(),
                                   [](const auto& a, const auto& b) {
                                       return a.second < b.second;
                                   });
        return it->second;
    }

    /**
     * @brief Get the average label value
     */
    std::optional<double> average_label() const {
        if (labels_.empty()) return std::nullopt;
        double sum = 0.0;
        for (const auto& [id, label] : labels_) {
            sum += label;
        }
        return sum / labels_.size();
    }

private:
    std::unordered_map<SimplexID, double> labels_;
};

} // namespace cebu
