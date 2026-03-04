#ifndef CEBU_TIMELINE_H
#define CEBU_TIMELINE_H

#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>

namespace cebu {

/// Represents a timeline for narrative events
class Timeline {
public:
    /// Construct a timeline with optional bounds
    explicit Timeline(double start = 0.0, double end = 1.0) {
        set_bounds(start, end);
    }

    /// Set timeline bounds
    /// @throws std::invalid_argument if start >= end or bounds are negative
    void set_bounds(double start, double end) {
        if (start < 0.0 || end < 0.0) {
            throw std::invalid_argument("Timeline bounds cannot be negative");
        }
        if (start >= end) {
            throw std::invalid_argument("Start time must be < end time");
        }
        start_time_ = start;
        end_time_ = end;
    }

    /// Get current timeline bounds
    std::pair<double, double> get_bounds() const {
        return {start_time_, end_time_};
    }

    /// Check if a timestamp is within timeline bounds
    bool contains(double timestamp) const {
        return timestamp >= start_time_ && timestamp <= end_time_;
    }

    /// Get total duration of timeline
    double duration() const {
        return end_time_ - start_time_;
    }

    /// Add a milestone at a specific time
    /// @throws std::invalid_argument if timestamp is outside bounds
    void add_milestone(double time, const std::string& label) {
        if (!contains(time)) {
            throw std::invalid_argument(
                "Milestone time " + std::to_string(time) + 
                " is outside timeline bounds [" + std::to_string(start_time_) + 
                ", " + std::to_string(end_time_) + "]"
            );
        }

        // Check if milestone already exists at this time
        auto it = std::find_if(milestones_.begin(), milestones_.end(),
                              [time](const auto& m) { return m.first == time; });
        if (it != milestones_.end()) {
            it->second = label;  // Update existing milestone
        } else {
            // Insert and maintain sorted order
            milestones_.push_back({time, label});
            std::sort(milestones_.begin(), milestones_.end(),
                     [](const auto& a, const auto& b) { return a.first < b.first; });
        }
    }

    /// Get all milestones sorted by time
    std::vector<std::pair<double, std::string>> get_milestones() const {
        return milestones_;
    }

    /// Get milestones in a time range
    /// @throws std::invalid_argument if start > end
    std::vector<std::pair<double, std::string>> get_milestones_in_range(double start, double end) const {
        if (start > end) {
            throw std::invalid_argument("Start time must be <= end time");
        }

        std::vector<std::pair<double, std::string>> result;
        for (const auto& milestone : milestones_) {
            if (milestone.first >= start && milestone.first <= end) {
                result.push_back(milestone);
            }
        }
        return result;
    }

    /// Get milestone at specific time, or nullptr if not found
    const std::string* get_milestone_at(double time) const {
        auto it = std::find_if(milestones_.begin(), milestones_.end(),
                              [time](const auto& m) { return m.first == time; });
        if (it != milestones_.end()) {
            return &(it->second);
        }
        return nullptr;
    }

    /// Remove milestone at specific time
    /// @returns true if milestone was found and removed, false otherwise
    bool remove_milestone(double time) {
        auto it = std::find_if(milestones_.begin(), milestones_.end(),
                              [time](const auto& m) { return m.first == time; });
        if (it != milestones_.end()) {
            milestones_.erase(it);
            return true;
        }
        return false;
    }

    /// Clear all milestones
    void clear_milestones() {
        milestones_.clear();
    }

    /// Get number of milestones
    size_t milestone_count() const {
        return milestones_.size();
    }

    /// Get next milestone time after given time
    /// @returns nullptr if no milestone after given time
    const std::string* get_next_milestone(double time) const {
        for (const auto& milestone : milestones_) {
            if (milestone.first > time) {
                return &(milestone.second);
            }
        }
        return nullptr;
    }

    /// Get previous milestone time before given time
    /// @returns nullptr if no milestone before given time
    const std::string* get_previous_milestone(double time) const {
        const std::string* result = nullptr;
        for (const auto& milestone : milestones_) {
            if (milestone.first < time) {
                result = &(milestone.second);
            } else {
                break;
            }
        }
        return result;
    }

private:
    double start_time_;
    double end_time_;
    std::vector<std::pair<double, std::string>> milestones_;
};

} // namespace cebu

#endif // CEBU_TIMELINE_H
