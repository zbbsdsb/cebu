#pragma once

#include <chrono>
#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <map>
#include <memory>

namespace cebu {

/**
 * @brief Timing utilities for performance measurement
 */
class Timer {
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;
    using Duration = std::chrono::microseconds;

    /**
     * @brief Start the timer
     */
    void start() {
        start_time_ = Clock::now();
    }

    /**
     * @brief Stop the timer
     */
    void stop() {
        end_time_ = Clock::now();
    }

    /**
     * @brief Get elapsed time in milliseconds
     */
    double elapsed_ms() const {
        auto duration = std::chrono::duration_cast<Duration>(end_time_ - start_time_);
        return duration.count() / 1000.0;
    }

    /**
     * @brief Get elapsed time in microseconds
     */
    double elapsed_us() const {
        auto duration = std::chrono::duration_cast<Duration>(end_time_ - start_time_);
        return static_cast<double>(duration.count());
    }

    /**
     * @brief Get elapsed time in seconds
     */
    double elapsed_s() const {
        return elapsed_ms() / 1000.0;
    }

private:
    TimePoint start_time_;
    TimePoint end_time_;
};

/**
 * @brief Statistical analysis of benchmark results
 */
struct BenchmarkStats {
    double mean;        ///< Mean execution time (ms)
    double std_dev;     ///< Standard deviation (ms)
    double min;         ///< Minimum time (ms)
    double max;         ///< Maximum time (ms)
    double median;      ///< Median time (ms)
    double p25;         ///< 25th percentile (ms)
    double p75;         ///< 75th percentile (ms)
    size_t iterations;   ///< Number of iterations
    double ops_per_sec; ///< Operations per second

    /**
     * @brief Format stats as string
     */
    std::string to_string() const;
};

/**
 * @brief Result of a single benchmark run
 */
struct BenchmarkResult {
    std::string name;           ///< Benchmark name
    BenchmarkStats stats;       ///< Statistical results
    std::vector<double> times;  ///< Individual run times (ms)

    /**
     * @brief Format result as string
     */
    std::string to_string() const;
};

/**
 * @brief Configuration for benchmark execution
 */
struct BenchmarkConfig {
    size_t warmup_iterations{3};      ///< Number of warmup runs
    size_t min_iterations{10};        ///< Minimum number of iterations
    size_t max_iterations{1000};      ///< Maximum number of iterations
    double min_time_ms{100.0};       ///< Minimum total time to run (ms)
    double max_time_ms{5000.0};      ///< Maximum total time to run (ms)
    bool verbose{false};              ///< Verbose output
};

/**
 * @brief Base class for benchmarks
 */
class Benchmark {
public:
    using SetupFunc = std::function<void()>;
    using RunFunc = std::function<void()>;
    using TeardownFunc = std::function<void()>;

    explicit Benchmark(const std::string& name);
    virtual ~Benchmark() = default;

    /**
     * @brief Set setup function
     */
    Benchmark& setup(SetupFunc func) {
        setup_func_ = std::move(func);
        return *this;
    }

    /**
     * @brief Set run function
     */
    Benchmark& run(RunFunc func) {
        run_func_ = std::move(func);
        return *this;
    }

    /**
     * @brief Set teardown function
     */
    Benchmark& teardown(TeardownFunc func) {
        teardown_func_ = std::move(func);
        return *this;
    }

    /**
     * @brief Run the benchmark with given config
     */
    BenchmarkResult execute(const BenchmarkConfig& config = BenchmarkConfig());

    /**
     * @brief Get benchmark name
     */
    const std::string& name() const { return name_; }

protected:
    std::string name_;
    SetupFunc setup_func_;
    RunFunc run_func_;
    TeardownFunc teardown_func_;
};

/**
 * @brief Benchmark runner that manages multiple benchmarks
 */
class BenchmarkRunner {
public:
    BenchmarkRunner() = default;
    ~BenchmarkRunner() = default;

    /**
     * @brief Add a benchmark to the runner
     */
    void add_benchmark(std::unique_ptr<Benchmark> benchmark);

    /**
     * @brief Run all benchmarks with given config
     */
    std::vector<BenchmarkResult> run_all(const BenchmarkConfig& config = BenchmarkConfig());

    /**
     * @brief Run a specific benchmark by name
     */
    BenchmarkResult run_benchmark(const std::string& name, 
                                const BenchmarkConfig& config = BenchmarkConfig());

    /**
     * @brief Compare all benchmarks results
     */
    void compare_results(const std::vector<BenchmarkResult>& results) const;

    /**
     * @brief Generate markdown report
     */
    std::string generate_report(const std::vector<BenchmarkResult>& results) const;

    /**
     * @brief Get number of registered benchmarks
     */
    size_t benchmark_count() const { return benchmarks_.size(); }

private:
    std::vector<std::unique_ptr<Benchmark>> benchmarks_;
};

/**
 * @brief Convenience functions for common benchmark patterns
 */
namespace benchmark_utils {

/**
 * @brief Measure execution time of a function
 */
template<typename Func>
double measure_time_ms(Func&& func, size_t iterations = 1) {
    Timer timer;
    timer.start();
    for (size_t i = 0; i < iterations; ++i) {
        func();
    }
    timer.stop();
    return timer.elapsed_ms();
}

/**
 * @brief Calculate statistics from a vector of times
 */
BenchmarkStats calculate_stats(const std::vector<double>& times);

/**
 * @brief Calculate percentiles
 */
double calculate_percentile(const std::vector<double>& values, double percentile);

/**
 * @brief Format time with appropriate units
 */
std::string format_time(double time_ms);

/**
 * @brief Format number with thousands separator
 */
std::string format_number(size_t value);

} // namespace benchmark_utils

/**
 * @brief Global benchmark runner instance
 */
class GlobalBenchmarkRunner {
public:
    static BenchmarkRunner& instance();
    static void add_benchmark(std::unique_ptr<Benchmark> benchmark);
    static std::vector<BenchmarkResult> run_all(const BenchmarkConfig& config);
    static void shutdown();

private:
    static std::unique_ptr<BenchmarkRunner> instance_;
    static bool initialized_;
};

} // namespace cebu
