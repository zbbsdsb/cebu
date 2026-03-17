#pragma once

#include <chrono>
#include <string>
#include <unordered_map>
#include <mutex>
#include <vector>
#include <atomic>

namespace cebu {

/**
 * @brief Timer for measuring elapsed time
 */
class ProfilerTimer {
public:
    using Clock = std::chrono::high_resolution_clock;

    /**
     * @brief Start timer
     */
    void start() {
        start_time_ = Clock::now();
    }

    /**
     * @brief Stop timer and return elapsed time in milliseconds
     */
    double stop() {
        auto end_time = Clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            end_time - start_time_);
        return duration.count() / 1000.0;
    }

    /**
     * @brief Get elapsed time in milliseconds without stopping
     */
    double elapsed_ms() const {
        auto now = Clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            now - start_time_);
        return duration.count() / 1000.0;
    }

private:
    Clock::time_point start_time_;
};

/**
 * @brief Statistics for a single timer
 */
struct TimerStats {
    size_t count{0};        ///< Number of times timer was stopped
    double total{0.0};       ///< Total time (ms)
    double min{1e9};        ///< Minimum time (ms)
    double max{0.0};        ///< Maximum time (ms)

    /**
     * @brief Calculate mean time
     */
    double mean() const {
        return (count > 0) ? (total / count) : 0.0;
    }

    /**
     * @brief Format stats as string
     */
    std::string to_string() const;
};

/**
 * @brief Performance profiler for measuring and tracking execution time
 */
class Profiler {
public:
    /**
     * @brief Constructor
     */
    Profiler() = default;

    /**
     * @brief Destructor - prints statistics automatically
     */
    ~Profiler();

    /**
     * @brief Start timing a named operation
     * @param name Name of the operation to time
     * @return Timer that will record time when stopped
     */
    ProfilerTimer start_timer(const std::string& name);

    /**
     * @brief Manually record time for a named operation
     * @param name Name of the operation
     * @param time_ms Time in milliseconds
     */
    void record(const std::string& name, double time_ms);

    /**
     * @brief Increment a named counter
     * @param name Name of the counter
     * @param count Amount to increment (default: 1)
     */
    void increment_counter(const std::string& name, size_t count = 1);

    /**
     * @brief Set a named counter to a specific value
     * @param name Name of the counter
     * @param value Value to set
     */
    void set_counter(const std::string& name, size_t value);

    /**
     * @brief Get timer statistics for a named operation
     * @param name Name of the operation
     * @return Statistics or default if not found
     */
    TimerStats get_timer_stats(const std::string& name) const;

    /**
     * @brief Get counter value for a named counter
     * @param name Name of the counter
     * @return Counter value or 0 if not found
     */
    size_t get_counter(const std::string& name) const;

    /**
     * @brief Print all statistics to stdout
     */
    void print_stats() const;

    /**
     * @brief Reset all timers and counters
     */
    void reset();

    /**
     * @brief Get all timer names
     */
    std::vector<std::string> get_timer_names() const;

    /**
     * @brief Get all counter names
     */
    std::vector<std::string> get_counter_names() const;

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, TimerStats> timers_;
    std::unordered_map<std::string, size_t> counters_;
};

/**
 * @brief RAII helper for automatic timing
 */
class ScopedTimer {
public:
    /**
     * @brief Constructor - starts timer
     * @param profiler Profiler to use
     * @param name Name of the operation
     */
    ScopedTimer(Profiler& profiler, const std::string& name)
        : profiler_(profiler), name_(name) {
        timer_.start();
    }

    /**
     * @brief Destructor - stops timer and records time
     */
    ~ScopedTimer() {
        double time = timer_.stop();
        profiler_.record(name_, time);
    }

private:
    Profiler& profiler_;
    std::string name_;
    ProfilerTimer timer_;
};

/**
 * @brief Global profiler instance
 */
class GlobalProfiler {
public:
    static Profiler& instance();
    static void reset();
    static void print_stats();

private:
    static std::unique_ptr<Profiler> instance_;
    static std::mutex mutex_;
};

/**
 * @brief Convenience macros for profiling
 */
#define PROFILER_START(name) \
    auto _profiler_timer_##__LINE__ = ::cebu::GlobalProfiler::instance().start_timer(name)

#define PROFILER_STOP(name, time) \
    ::cebu::GlobalProfiler::instance().record(name, time)

#define PROFILER_SCOPE(name) \
    ::cebu::ScopedTimer _scoped_timer_##__LINE__(::cebu::GlobalProfiler::instance(), name)

#define PROFILER_INC(name) \
    ::cebu::GlobalProfiler::instance().increment_counter(name)

#define PROFILER_SET(name, value) \
    ::cebu::GlobalProfiler::instance().set_counter(name, value)

} // namespace cebu
