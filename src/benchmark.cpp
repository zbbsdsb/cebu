#include "cebu/benchmark.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <stdexcept>

namespace cebu {

// ========================================
// BenchmarkStats Implementation
// ========================================

std::string BenchmarkStats::to_string() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(3);
    oss << "Mean: " << mean << " ms, ";
    oss << "StdDev: " << std_dev << " ms, ";
    oss << "Min: " << min << " ms, ";
    oss << "Max: " << max << " ms, ";
    oss << "Median: " << median << " ms";
    return oss.str();
}

// ========================================
// BenchmarkResult Implementation
// ========================================

std::string BenchmarkResult::to_string() const {
    std::ostringstream oss;
    oss << name << ":\n";
    oss << "  " << stats.to_string() << "\n";
    oss << "  Iterations: " << stats.iterations << "\n";
    oss << "  Ops/sec: " << std::fixed << std::setprecision(2) 
        << stats.ops_per_sec;
    return oss.str();
}

// ========================================
// Benchmark Implementation
// ========================================

Benchmark::Benchmark(const std::string& name)
    : name_(name) {
}

BenchmarkResult Benchmark::execute(const BenchmarkConfig& config) {
    BenchmarkResult result;
    result.name = name_;

    if (!run_func_) {
        throw std::runtime_error("Benchmark run function not set");
    }

    // Warmup runs
    if (config.verbose) {
        std::cout << "[Warmup] " << name_ << " (" 
                  << config.warmup_iterations << " iterations)" << std::endl;
    }

    for (size_t i = 0; i < config.warmup_iterations; ++i) {
        if (setup_func_) setup_func_();
        run_func_();
        if (teardown_func_) teardown_func_();
    }

    // Determine number of iterations
    size_t iterations = config.min_iterations;
    double total_time = 0.0;

    if (config.verbose) {
        std::cout << "[Running] " << name_ << std::endl;
    }

    // Main benchmark loop
    while (iterations < config.max_iterations && total_time < config.min_time_ms) {
        // Run benchmark
        for (size_t i = 0; i < iterations; ++i) {
            if (setup_func_) setup_func_();

            Timer timer;
            timer.start();
            run_func_();
            timer.stop();

            result.times.push_back(timer.elapsed_ms());

            if (teardown_func_) teardown_func_();
        }

        total_time = std::accumulate(result.times.begin(), result.times.end(), 0.0);

        // Check if we've run long enough
        if (total_time >= config.min_time_ms) {
            break;
        }

        // Double iterations for next round
        iterations = std::min(iterations * 2, config.max_iterations);
    }

    // Calculate statistics
    result.stats = benchmark_utils::calculate_stats(result.times);
    result.stats.iterations = result.times.size();

    // Calculate ops/sec (assuming 1 op per run)
    if (result.stats.mean > 0) {
        result.stats.ops_per_sec = 1000.0 / result.stats.mean;
    }

    return result;
}

// ========================================
// BenchmarkRunner Implementation
// ========================================

void BenchmarkRunner::add_benchmark(std::unique_ptr<Benchmark> benchmark) {
    benchmarks_.push_back(std::move(benchmark));
}

std::vector<BenchmarkResult> BenchmarkRunner::run_all(const BenchmarkConfig& config) {
    std::vector<BenchmarkResult> results;

    std::cout << "\n========================================" << std::endl;
    std::cout << "  Running " << benchmarks_.size() << " benchmarks" << std::endl;
    std::cout << "========================================\n" << std::endl;

    for (auto& benchmark : benchmarks_) {
        try {
            auto result = benchmark->execute(config);
            results.push_back(result);
            std::cout << result.to_string() << "\n" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "[ERROR] " << benchmark->name() 
                      << ": " << e.what() << std::endl;
        }
    }

    std::cout << "========================================" << std::endl;
    std::cout << "  All benchmarks completed" << std::endl;
    std::cout << "========================================\n" << std::endl;

    return results;
}

BenchmarkResult BenchmarkRunner::run_benchmark(const std::string& name,
                                             const BenchmarkConfig& config) {
    auto it = std::find_if(benchmarks_.begin(), benchmarks_.end(),
                           [&name](const auto& bench) {
                               return bench->name() == name;
                           });

    if (it == benchmarks_.end()) {
        throw std::runtime_error("Benchmark not found: " + name);
    }

    return (*it)->execute(config);
}

void BenchmarkRunner::compare_results(const std::vector<BenchmarkResult>& results) const {
    if (results.empty()) {
        std::cout << "No results to compare." << std::endl;
        return;
    }

    std::cout << "\n========================================" << std::endl;
    std::cout << "  Benchmark Comparison" << std::endl;
    std::cout << "========================================\n" << std::endl;

    std::cout << std::left << std::setw(30) << "Benchmark"
              << std::right << std::setw(12) << "Mean (ms)"
              << std::setw(12) << "StdDev (ms)"
              << std::setw(12) << "Ops/sec"
              << std::setw(10) << "Relative" << std::endl;
    std::cout << std::string(76, '-') << std::endl;

    double baseline = results[0].stats.mean;
    for (const auto& result : results) {
        double relative = result.stats.mean / baseline;
        std::cout << std::left << std::setw(30) << result.name
                  << std::right << std::setw(12) 
                  << std::fixed << std::setprecision(3) << result.stats.mean
                  << std::setw(12) << result.stats.std_dev
                  << std::setw(12) << std::fixed << std::setprecision(0) 
                  << result.stats.ops_per_sec
                  << std::setw(10) << std::fixed << std::setprecision(2) 
                  << relative << "x" << std::endl;
    }
}

std::string BenchmarkRunner::generate_report(const std::vector<BenchmarkResult>& results) const {
    std::ostringstream oss;

    oss << "# Benchmark Report\n\n";
    oss << "Generated: " << std::chrono::system_clock::now().time_since_epoch().count() << "\n\n";

    oss << "## Summary\n\n";
    oss << "| Benchmark | Mean (ms) | StdDev (ms) | Min (ms) | Max (ms) | Ops/sec |\n";
    oss << "|-----------|-----------|-------------|----------|----------|----------|\n";

    for (const auto& result : results) {
        oss << "| " << result.name
            << " | " << std::fixed << std::setprecision(3) << result.stats.mean
            << " | " << result.stats.std_dev
            << " | " << result.stats.min
            << " | " << result.stats.max
            << " | " << std::fixed << std::setprecision(0) << result.stats.ops_per_sec
            << " |\n";
    }

    oss << "\n## Detailed Results\n\n";
    for (const auto& result : results) {
        oss << "### " << result.name << "\n\n";
        oss << "- **Iterations**: " << result.stats.iterations << "\n";
        oss << "- **Mean**: " << result.stats.mean << " ms\n";
        oss << "- **StdDev**: " << result.stats.std_dev << " ms\n";
        oss << "- **Min**: " << result.stats.min << " ms\n";
        oss << "- **Max**: " << result.stats.max << " ms\n";
        oss << "- **Median**: " << result.stats.median << " ms\n";
        oss << "- **P25**: " << result.stats.p25 << " ms\n";
        oss << "- **P75**: " << result.stats.p75 << " ms\n";
        oss << "- **Ops/sec**: " << result.stats.ops_per_sec << "\n\n";
    }

    return oss.str();
}

// ========================================
// Benchmark Utils Implementation
// ========================================

namespace benchmark_utils {

BenchmarkStats calculate_stats(const std::vector<double>& times) {
    BenchmarkStats stats;

    if (times.empty()) {
        return stats;
    }

    // Calculate mean
    stats.mean = std::accumulate(times.begin(), times.end(), 0.0) / times.size();

    // Calculate standard deviation
    double sum_sq_diff = 0.0;
    for (double t : times) {
        double diff = t - stats.mean;
        sum_sq_diff += diff * diff;
    }
    stats.std_dev = std::sqrt(sum_sq_diff / times.size());

    // Calculate min and max
    stats.min = *std::min_element(times.begin(), times.end());
    stats.max = *std::max_element(times.begin(), times.end());

    // Sort for percentile calculations
    std::vector<double> sorted = times;
    std::sort(sorted.begin(), sorted.end());

    // Calculate percentiles
    stats.median = calculate_percentile(sorted, 0.5);
    stats.p25 = calculate_percentile(sorted, 0.25);
    stats.p75 = calculate_percentile(sorted, 0.75);

    stats.iterations = times.size();
    stats.ops_per_sec = 1000.0 / stats.mean;

    return stats;
}

double calculate_percentile(const std::vector<double>& values, double percentile) {
    if (values.empty()) {
        return 0.0;
    }

    size_t index = static_cast<size_t>(percentile * (values.size() - 1));
    return values[index];
}

std::string format_time(double time_ms) {
    if (time_ms < 1.0) {
        return std::to_string(time_ms * 1000.0) + " us";
    } else if (time_ms < 1000.0) {
        return std::to_string(time_ms) + " ms";
    } else {
        return std::to_string(time_ms / 1000.0) + " s";
    }
}

std::string format_number(size_t value) {
    std::string s = std::to_string(value);
    int insertPosition = static_cast<int>(s.length()) - 3;

    while (insertPosition > 0) {
        s.insert(insertPosition, ",");
        insertPosition -= 3;
    }

    return s;
}

} // namespace benchmark_utils

// ========================================
// GlobalBenchmarkRunner Implementation
// ========================================

std::unique_ptr<BenchmarkRunner> GlobalBenchmarkRunner::instance_ = nullptr;
bool GlobalBenchmarkRunner::initialized_ = false;

BenchmarkRunner& GlobalBenchmarkRunner::instance() {
    if (!instance_) {
        instance_ = std::make_unique<BenchmarkRunner>();
        initialized_ = true;
    }
    return *instance_;
}

void GlobalBenchmarkRunner::add_benchmark(std::unique_ptr<Benchmark> benchmark) {
    instance().add_benchmark(std::move(benchmark));
}

std::vector<BenchmarkResult> GlobalBenchmarkRunner::run_all(const BenchmarkConfig& config) {
    return instance().run_all(config);
}

void GlobalBenchmarkRunner::shutdown() {
    instance_.reset();
    initialized_ = false;
}

} // namespace cebu
