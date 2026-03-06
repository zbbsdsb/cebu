#include "cebu/parallel_executor.h"
#include <stdexcept>
#include <iostream>
#include <algorithm>

namespace cebu {

// ========================================
// ParallelExecutor 实现
// ========================================

ParallelExecutor::ParallelExecutor(size_t num_threads, ParallelStrategy strategy)
    : stop_(false), active_tasks_(0), strategy_(strategy) {
    // 如果未指定线程数，使用硬件并发数
    const size_t actual_threads = (num_threads == 0) ? 
        std::thread::hardware_concurrency() : num_threads;
    
    // 确保至少有一个线程
    const size_t worker_count = std::max<size_t>(actual_threads, 1);
    
    workers_.reserve(worker_count);
    
    // 创建工作线程
    for (size_t i = 0; i < worker_count; ++i) {
        workers_.emplace_back(&ParallelExecutor::worker_thread, this);
    }
    
    std::cout << "[ParallelExecutor] Initialized with " 
              << worker_count << " worker threads" << std::endl;
}

ParallelExecutor::~ParallelExecutor() {
    shutdown();
}

void ParallelExecutor::shutdown() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        stop_ = true;
    }
    
    condition_.notify_all();
    
    // 等待所有工作线程完成
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    
    workers_.clear();
}

void ParallelExecutor::wait_for_all() {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    finished_.wait(lock, [this]() {
        return tasks_.empty() && active_tasks_.load(std::memory_order_relaxed) == 0;
    });
}

void ParallelExecutor::worker_thread() {
    while (true) {
        std::function<void()> task;
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            
            // 等待任务或停止信号
            condition_.wait(lock, [this]() {
                return stop_ || !tasks_.empty();
            });
            
            // 如果收到停止信号且队列为空，退出线程
            if (stop_ && tasks_.empty()) {
                return;
            }
            
            // 取出任务
            task = std::move(tasks_.front());
            tasks_.pop();
        }
        
        // 执行任务
        task();
        
        // 减少活动任务计数
        size_t prev = active_tasks_.fetch_sub(1, std::memory_order_relaxed);
        if (prev == 1) {
            finished_.notify_all();
        }
    }
}

size_t ParallelExecutor::calculate_chunk_size(size_t total_size) const {
    if (total_size == 0) return 1;
    
    const size_t num_threads = workers_.size();
    
    switch (strategy_) {
        case ParallelStrategy::Static:
            // 静态分区：均匀分配
            return std::max<size_t>(total_size / num_threads, 1);
            
        case ParallelStrategy::Dynamic:
            // 动态分区：较小的块大小，适合工作窃取
            return std::max<size_t>(total_size / (num_threads * 4), 1);
            
        case ParallelStrategy::Guided:
            // 引导式：递减的块大小
            return std::max<size_t>(total_size / (num_threads * 2), 1);
            
        default:
            return std::max<size_t>(total_size / num_threads, 1);
    }
}

// ========================================
// GlobalParallelExecutor 实现
// ========================================

std::unique_ptr<ParallelExecutor> GlobalParallelExecutor::executor_ = nullptr;
std::mutex GlobalParallelExecutor::mutex_;

ParallelExecutor& GlobalParallelExecutor::instance() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!executor_) {
        executor_ = std::make_unique<ParallelExecutor>();
    }
    
    return *executor_;
}

void GlobalParallelExecutor::initialize(size_t num_threads) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!executor_) {
        executor_ = std::make_unique<ParallelExecutor>(num_threads);
    }
}

void GlobalParallelExecutor::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    executor_.reset();
}

} // namespace cebu
