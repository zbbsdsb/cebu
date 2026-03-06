#include "cebu/profiler.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

namespace cebu {

// ========================================
// TimerStats Implementation
// ========================================

std::string TimerStats::to_string() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(3);
    oss << "count=" << count << ", ";
    oss << "total=" << total << "ms, ";
    oss << "mean=" << mean() << "ms, ";
    oss << "min=" << min << "ms, ";
    oss << "max=" << max << "ms";
    return oss.str();
}

// ========================================
// Profiler Implementation
// ========================================

Profiler::~Profiler() {
    // Print statistics on destruction
    print_stats();
}

ProfilerTimer Profiler::start_timer(const std::string& name) {
    ProfilerTimer timer;
    timer.start();
    return timer;
}

void Profiler::record(const std::string& name, double time_ms) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto& stats = timers_[name];
    stats.count++;
    stats.total += time_ms;
    stats.min = std::min(stats.min, time_ms);
    stats.max = std::max(stats.max, time_ms);
}

void Profiler::increment_counter(const std::string& name, size_t count) {
    std::lock_guard<std::mutex> lock(mutex_);
    counters_[name] += count;
}

void Profiler::set_counter(const std::string& name, size_t value) {
    std::lock_guard<std::mutex> lock(mutex_);
    counters_[name] = value;
}

TimerStats Profiler::get_timer_stats(const std::string& name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = timers_.find(name);
    if (it != timers_.end()) {
        return it->second;
    }
    
    return TimerStats{};  // Return default stats if not found
}

size_t Profiler::get_counter(const std::string& name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = counters_.find(name);
    if (it != counters_.end()) {
        return it->second;
    }
    
    return 0;  // Return 0 if not found
}

void Profiler::print_stats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (timers_.empty() && counters_.empty()) {
        return;
    }
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "  Profiler Statistics" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Print timers
    if (!timers_.empty()) {
        std::cout << "\nTimers:\n";
        std::cout << std::string(76, '-') << std::endl;
        
        // Sort by total time (descending)
        std::vector<std::pair<std::string, TimerStats>> sorted_timers(
            timers_.begin(), timers_.end());
        std::sort(sorted_timers.begin(), sorted_timers.end(),
                 [](const auto& a, const auto& b) {
                     return a.second.total > b.second.total;
                 });
        
        for (const auto& [name, stats] : sorted_timers) {
            std::cout << std::left << std::setw(30) << name
                      << std::right << std::setw(10) << stats.count
                      << std::setw(12) << std::fixed << std::setprecision(3) 
                      << stats.total << "ms"
                      << std::setw(12) << stats.mean() << "ms"
                      << std::setw(12) << stats.min << "ms"
                      << std::setw(12) << stats.max << "ms"
                      << std::endl;
        }
    }
    
    // Print counters
    if (!counters_.empty()) {
        std::cout << "\nCounters:\n";
        std::cout << std::string(76, '-') << std::endl;
        
        for (const auto& [name, count] : counters_) {
            std::cout << std::left << std::setw(30) << name
                      << std::right << std::setw(10) << count
                      << std::endl;
        }
    }
    
    std::cout << "========================================\n" << std::endl;
}

void Profiler::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    timers_.clear();
    counters_.clear();
}

std::vector<std::string> Profiler::get_timer_names() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<std::string> names;
    names.reserve(timers_.size());
    
    for (const auto& [name, _] : timers_) {
        names.push_back(name);
    }
    
    return names;
}

std::vector<std::string> Profiler::get_counter_names() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<std::string> names;
    names.reserve(counters_.size());
    
    for (const auto& [name, _] : counters_) {
        names.push_back(name);
    }
    
    return names;
}

// ========================================
// GlobalProfiler Implementation
// ========================================

std::unique_ptr<Profiler> GlobalProfiler::instance_ = nullptr;
std::mutex GlobalProfiler::mutex_;

Profiler& GlobalProfiler::instance() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!instance_) {
        instance_ = std::make_unique<Profiler>();
    }
    
    return *instance_;
}

void GlobalProfiler::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (instance_) {
        instance_->reset();
    }
}

void GlobalProfiler::print_stats() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (instance_) {
        instance_->print_stats();
    }
}

} // namespace cebu
