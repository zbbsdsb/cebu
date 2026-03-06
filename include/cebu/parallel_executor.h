#pragma once

#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <future>
#include <functional>
#include <atomic>
#include <memory>
#include <type_traits>
#include <algorithm>
#include <numeric>

namespace cebu {

/**
 * @brief 并行执行策略
 */
enum class ParallelStrategy {
    Static,     ///< 静态分区，均匀分配
    Dynamic,    ///< 动态分区，工作窃取
    Guided      ///< 引导式分区，递减块大小
};

/**
 * @brief 任务状态
 */
enum class TaskStatus {
    Pending,    ///< 等待执行
    Running,    ///< 正在执行
    Completed,  ///< 已完成
    Failed      ///< 执行失败
};

/**
 * @brief 并行任务
 * 
 * 表示一个可以并行执行的任务单元
 */
class Task {
public:
    using TaskId = uint64_t;
    using TaskFunction = std::function<void()>;

    Task() : id_(generate_id()), status_(TaskStatus::Pending) {}
    
    explicit Task(TaskFunction func) 
        : id_(generate_id()), 
          status_(TaskStatus::Pending),
          function_(std::move(func)) {}

    TaskId id() const { return id_; }
    TaskStatus status() const { return status_; }
    
    void set_status(TaskStatus status) { 
        status_ = status; 
    }
    
    void add_dependency(TaskId id) {
        dependencies_.push_back(id);
    }
    
    const std::vector<TaskId>& dependencies() const {
        return dependencies_;
    }
    
    void execute() {
        if (function_) {
            function_();
        }
    }
    
    void set_function(TaskFunction func) {
        function_ = std::move(func);
    }
    
    bool is_ready() const {
        return status_ == TaskStatus::Pending;
    }

private:
    static TaskId generate_id() {
        static std::atomic<TaskId> counter{0};
        return ++counter;
    }

    TaskId id_;
    TaskStatus status_;
    std::vector<TaskId> dependencies_;
    TaskFunction function_;
};

/**
 * @brief 并行执行器
 * 
 * 提供高效的并行任务执行能力，包括线程池、任务调度和并行算法
 */
class ParallelExecutor {
public:
    /**
     * @brief 构造并行执行器
     * @param num_threads 工作线程数，0表示使用硬件并发数
     * @param strategy 并行策略
     */
    explicit ParallelExecutor(size_t num_threads = 0, 
                             ParallelStrategy strategy = ParallelStrategy::Dynamic);
    
    /**
     * @brief 析构函数，等待所有任务完成
     */
    ~ParallelExecutor();
    
    // 禁止拷贝
    ParallelExecutor(const ParallelExecutor&) = delete;
    ParallelExecutor& operator=(const ParallelExecutor&) = delete;
    
    /**
     * @brief 提交任务到队列
     * @tparam F 函数类型
     * @tparam Args 参数类型
     * @param f 函数
     * @param args 参数
     * @return 任务的 Future 对象
     */
    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type>;
    
    /**
     * @brief 并行 for 循环
     * @tparam Iterator 迭代器类型
     * @tparam Function 函数类型
     * @param begin 起始迭代器
     * @param end 结束迭代器
     * @param func 函数
     * @param chunk_size 分块大小（0表示自动）
     */
    template<typename Iterator, typename Function>
    void parallel_for(Iterator begin, Iterator end, 
                     Function&& func, 
                     size_t chunk_size = 0);
    
    /**
     * @brief 并行 for 循环（索引范围）
     * @tparam Function 函数类型
     * @param start 起始索引
     * @param end 结束索引
     * @param func 函数
     * @param chunk_size 分块大小
     */
    template<typename Function>
    void parallel_for(size_t start, size_t end, 
                     Function&& func, 
                     size_t chunk_size = 0);
    
    /**
     * @brief 并行归约
     * @tparam Iterator 迭代器类型
     * @tparam Function 归约函数类型
     * @tparam T 结果类型
     * @param begin 起始迭代器
     * @param end 结束迭代器
     * @param func 归约函数
     * @param init 初始值
     * @return 归约结果
     */
    template<typename Iterator, typename Function, typename T>
    T parallel_reduce(Iterator begin, Iterator end,
                     Function&& func,
                     T init);
    
    /**
     * @brief 并行映射
     * @tparam Iterator 输入迭代器类型
     * @tparam OutputIterator 输出迭代器类型
     * @tparam Function 映射函数类型
     * @param begin 起始迭代器
     * @param end 结束迭代器
     * @param out 输出迭代器
     * @param func 映射函数
     */
    template<typename Iterator, typename OutputIterator, typename Function>
    void parallel_map(Iterator begin, Iterator end,
                     OutputIterator out,
                     Function&& func);
    
    /**
     * @brief 等待所有任务完成
     */
    void wait_for_all();
    
    /**
     * @brief 获取活动任务数
     */
    size_t active_tasks() const {
        return active_tasks_.load(std::memory_order_relaxed);
    }
    
    /**
     * @brief 获取线程数
     */
    size_t thread_count() const {
        return workers_.size();
    }
    
    /**
     * @brief 设置并行策略
     */
    void set_strategy(ParallelStrategy strategy) {
        strategy_ = strategy;
    }
    
    /**
     * @brief 关闭线程池
     */
    void shutdown();

private:
    /**
     * @brief 工作线程主函数
     */
    void worker_thread();
    
    /**
     * @brief 计算分块大小
     */
    size_t calculate_chunk_size(size_t total_size) const;
    
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::condition_variable finished_;
    std::atomic<bool> stop_;
    std::atomic<size_t> active_tasks_;
    ParallelStrategy strategy_;
};

// ========================================
// 模板实现
// ========================================

template<typename F, typename... Args>
auto ParallelExecutor::enqueue(F&& f, Args&&... args) 
    -> std::future<typename std::result_of<F(Args...)>::type>
{
    using ReturnType = typename std::result_of<F(Args...)>::type;
    
    auto task = std::make_shared<std::packaged_task<ReturnType()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );
    
    std::future<ReturnType> result = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        if (stop_) {
            throw std::runtime_error("enqueue on stopped ParallelExecutor");
        }
        
        tasks_.emplace([task]() { (*task)(); });
        ++active_tasks_;
    }
    condition_.notify_one();
    
    return result;
}

template<typename Iterator, typename Function>
void ParallelExecutor::parallel_for(Iterator begin, Iterator end,
                                   Function&& func,
                                   size_t chunk_size) {
    const auto total_size = std::distance(begin, end);
    if (total_size <= 0) return;
    
    const auto actual_chunk_size = (chunk_size == 0) ? 
        calculate_chunk_size(total_size) : chunk_size;
    
    std::vector<std::future<void>> futures;
    
    auto it = begin;
    while (it != end) {
        auto chunk_end = it;
        std::advance(chunk_end, std::min(actual_chunk_size, 
                                         static_cast<size_t>(std::distance(it, end))));
        
        futures.push_back(enqueue([=]() {
            for (auto chunk_it = it; chunk_it != chunk_end; ++chunk_it) {
                func(*chunk_it);
            }
        }));
        
        it = chunk_end;
    }
    
    for (auto& f : futures) {
        f.wait();
    }
}

template<typename Function>
void ParallelExecutor::parallel_for(size_t start, size_t end,
                                   Function&& func,
                                   size_t chunk_size) {
    if (start >= end) return;
    
    const auto total_size = end - start;
    const auto actual_chunk_size = (chunk_size == 0) ? 
        calculate_chunk_size(total_size) : chunk_size;
    
    std::vector<std::future<void>> futures;
    
    for (size_t i = start; i < end; i += actual_chunk_size) {
        const auto chunk_end = std::min(i + actual_chunk_size, end);
        
        futures.push_back(enqueue([=]() {
            for (size_t j = i; j < chunk_end; ++j) {
                func(j);
            }
        }));
    }
    
    for (auto& f : futures) {
        f.wait();
    }
}

template<typename Iterator, typename Function, typename T>
T ParallelExecutor::parallel_reduce(Iterator begin, Iterator end,
                                   Function&& func,
                                   T init) {
    const auto total_size = std::distance(begin, end);
    if (total_size <= 0) return init;
    
    const auto chunk_size = calculate_chunk_size(total_size);
    
    std::vector<std::future<T>> futures;
    
    auto it = begin;
    while (it != end) {
        auto chunk_end = it;
        std::advance(chunk_end, std::min(chunk_size,
                                         static_cast<size_t>(std::distance(it, end))));
        
        futures.push_back(enqueue([=]() {
            T result = init;
            for (auto chunk_it = it; chunk_it != chunk_end; ++chunk_it) {
                result = func(result, *chunk_it);
            }
            return result;
        }));
        
        it = chunk_end;
    }
    
    T final_result = init;
    for (auto& f : futures) {
        final_result = func(final_result, f.get());
    }
    
    return final_result;
}

template<typename Iterator, typename OutputIterator, typename Function>
void ParallelExecutor::parallel_map(Iterator begin, Iterator end,
                                   OutputIterator out,
                                   Function&& func) {
    const auto total_size = std::distance(begin, end);
    if (total_size <= 0) return;
    
    const auto chunk_size = calculate_chunk_size(total_size);
    
    std::vector<std::future<void>> futures;
    
    auto it = begin;
    auto out_it = out;
    
    while (it != end) {
        auto chunk_end = it;
        std::advance(chunk_end, std::min(chunk_size,
                                         static_cast<size_t>(std::distance(it, end))));
        
        auto chunk_out = out_it;
        std::advance(out_it, std::distance(it, chunk_end));
        
        futures.push_back(enqueue([=]() {
            auto chunk_it = it;
            auto chunk_out_it = chunk_out;
            for (; chunk_it != chunk_end; ++chunk_it, ++chunk_out_it) {
                *chunk_out_it = func(*chunk_it);
            }
        }));
        
        it = chunk_end;
    }
    
    for (auto& f : futures) {
        f.wait();
    }
}

/**
 * @brief 全局并行执行器实例
 * 
 * 提供便捷的全局访问接口
 */
class GlobalParallelExecutor {
public:
    static ParallelExecutor& instance();
    static void initialize(size_t num_threads = 0);
    static void shutdown();

private:
    static std::unique_ptr<ParallelExecutor> executor_;
    static std::mutex mutex_;
};

/**
 * @brief 便捷函数：全局并行 for
 */
template<typename Iterator, typename Function>
void parallel_for(Iterator begin, Iterator end, Function&& func) {
    GlobalParallelExecutor::instance().parallel_for(begin, end, 
                                                    std::forward<Function>(func));
}

/**
 * @brief 便捷函数：全局并行 for（索引范围）
 */
template<typename Function>
void parallel_for(size_t start, size_t end, Function&& func) {
    GlobalParallelExecutor::instance().parallel_for(start, end, 
                                                    std::forward<Function>(func));
}

/**
 * @brief 便捷函数：全局并行归约
 */
template<typename Iterator, typename Function, typename T>
T parallel_reduce(Iterator begin, Iterator end, Function&& func, T init) {
    return GlobalParallelExecutor::instance().parallel_reduce(
        begin, end, std::forward<Function>(func), init);
}

/**
 * @brief 便捷函数：全局并行映射
 */
template<typename Iterator, typename OutputIterator, typename Function>
void parallel_map(Iterator begin, Iterator end, OutputIterator out, Function&& func) {
    GlobalParallelExecutor::instance().parallel_map(
        begin, end, out, std::forward<Function>(func));
}

} // namespace cebu
