#include "cebu/spatial_index.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <functional>

namespace cebu {

// ============================================================================
// SpatialIndex Implementation
// ============================================================================

SpatialIndex::SpatialIndex()
    : complex_(nullptr)
    , labeled_complex_(nullptr)
    , current_type_(SpatialIndexType::AUTO)
    , recommended_type_(SpatialIndexType::BVH)
    , cache_enabled_(false)
    , max_cache_size_(1000)
    , profiling_enabled_(false)
    , adaptive_optimization_enabled_(false) {}

SpatialIndex::SpatialIndex(const SimplicialComplex& complex)
    : complex_(&complex)
    , labeled_complex_(nullptr)
    , current_type_(SpatialIndexType::AUTO)
    , recommended_type_(SpatialIndexType::BVH)
    , cache_enabled_(false)
    , max_cache_size_(1000)
    , profiling_enabled_(false)
    , adaptive_optimization_enabled_(false) {
    build(complex);
}

SpatialIndex::SpatialIndex(const SimplicialComplexLabeled<>& complex)
    : complex_(nullptr)
    , labeled_complex_(&complex)
    , current_type_(SpatialIndexType::AUTO)
    , recommended_type_(SpatialIndexType::BVH)
    , cache_enabled_(false)
    , max_cache_size_(1000)
    , profiling_enabled_(false)
    , adaptive_optimization_enabled_(false) {
    build(complex);
}

SpatialIndex::~SpatialIndex() = default;

void SpatialIndex::build(const SimplicialComplex& complex) {
    complex_ = &complex;
    labeled_complex_ = nullptr;
    reset_metrics();

    auto start = std::chrono::high_resolution_clock::now();

    // Analyze data distribution
    analyze_data_distribution();

    // Build recommended index
    switch_index(recommended_type_);

    auto end = std::chrono::high_resolution_clock::now();
    metrics_.build_time_ms = std::chrono::duration<double>(end - start).count() * 1000.0;
}

void SpatialIndex::build(const SimplicialComplexLabeled<>& complex) {
    labeled_complex_ = &complex;
    complex_ = nullptr;
    reset_metrics();

    auto start = std::chrono::high_resolution_clock::now();

    // Analyze data distribution
    analyze_data_distribution();

    // Build recommended index
    switch_index(recommended_type_);

    auto end = std::chrono::high_resolution_clock::now();
    metrics_.build_time_ms = std::chrono::duration<double>(end - start).count() * 1000.0;
}

void SpatialIndex::rebuild() {
    rebuild(current_type_);
}

void SpatialIndex::rebuild(SpatialIndexType type) {
    set_index_type(type);
    clear_cache();
    
    if (complex_) {
        build(*complex_);
    } else if (labeled_complex_) {
        build(*labeled_complex_);
    }
}

void SpatialIndex::set_index_type(SpatialIndexType type) {
    current_type_ = type;
    if (type == SpatialIndexType::AUTO) {
        type = recommended_type_;
    }
}

SpatialIndexType SpatialIndex::get_index_type() const {
    return current_type_;
}

SpatialIndexType SpatialIndex::get_recommended_index_type() const {
    return recommended_type_;
}

bool SpatialIndex::contains(const std::array<double, 3>& point) const {
    return !query_point(point).empty();
}

std::vector<SimplexID> SpatialIndex::query_point(const std::array<double, 3>& point) const {
    auto start = std::chrono::high_resolution_clock::now();

    // Check cache
    if (cache_enabled_) {
        size_t hash = hash_query(point);
        auto it = query_cache_.find(hash);
        if (it != query_cache_.end()) {
            it->second.access_count++;
            it->second.timestamp = std::chrono::system_clock::now();
            return it->second.results;
        }
    }

    // Query using active index
    std::vector<SimplexID> results;
    size_t nodes_visited = 0;

    SpatialIndexType active_type = (current_type_ == SpatialIndexType::AUTO) ?
                                   recommended_type_ : current_type_;

    if (active_type == SpatialIndexType::BVH && bvh_) {
        results = bvh_->query_point(point);
    } else if (active_type == SpatialIndexType::OCTREE && octree_) {
        results = octree_->query_point(point);
    }

    // Cache results
    if (cache_enabled_ && !results.empty()) {
        size_t hash = hash_query(point);
        QueryCacheEntry entry;
        entry.results = results;
        entry.timestamp = std::chrono::system_clock::now();
        entry.access_count = 1;

        // Evict if cache is full
        if (query_cache_.size() >= max_cache_size_) {
            auto oldest = std::min_element(query_cache_.begin(), query_cache_.end(),
                [](const auto& a, const auto& b) {
                    return a.second.timestamp < b.second.timestamp;
                });
            query_cache_.erase(oldest);
        }
        query_cache_[hash] = entry;
    }

    auto end = std::chrono::high_resolution_clock::now();
    if (profiling_enabled_) {
        update_metrics(end - start, nodes_visited);
        query_counts_[QueryType::POINT]++;
    }

    return results;
}

std::vector<SimplexID> SpatialIndex::query_range(const std::array<double, 3>& min,
                                                 const std::array<double, 3>& max) const {
    auto start = std::chrono::high_resolution_clock::now();

    std::vector<SimplexID> results;

    SpatialIndexType active_type = (current_type_ == SpatialIndexType::AUTO) ?
                                   recommended_type_ : current_type_;

    if (active_type == SpatialIndexType::BVH && bvh_) {
        BoundingBox bbox(min, max, 0);
        BVHQueryResult bvh_result = bvh_->query_range(bbox);
        results = bvh_result.simplex_ids;
    } else if (active_type == SpatialIndexType::OCTREE && octree_) {
        OctreeQueryResult octree_result = octree_->query_range(min, max);
        results = octree_result.simplex_ids;
    }

    auto end = std::chrono::high_resolution_clock::now();
    if (profiling_enabled_) {
        update_metrics(end - start, 0);
        query_counts_[QueryType::RANGE]++;
    }

    return results;
}

std::vector<SimplexID> SpatialIndex::query_sphere(const std::array<double, 3>& center,
                                                   double radius) const {
    auto start = std::chrono::high_resolution_clock::now();

    std::vector<SimplexID> results;

    SpatialIndexType active_type = (current_type_ == SpatialIndexType::AUTO) ?
                                   recommended_type_ : current_type_;

    if (active_type == SpatialIndexType::BVH && bvh_) {
        BVHQueryResult bvh_result = bvh_->query_sphere(center, radius);
        results = bvh_result.simplex_ids;
    } else if (active_type == SpatialIndexType::OCTREE && octree_) {
        OctreeQueryResult octree_result = octree_->query_sphere(center, radius);
        results = octree_result.simplex_ids;
    }

    auto end = std::chrono::high_resolution_clock::now();
    if (profiling_enabled_) {
        update_metrics(end - start, 0);
        query_counts_[QueryType::SPHERE]++;
    }

    return results;
}

std::vector<SimplexID> SpatialIndex::nearest_neighbors(const std::array<double, 3>& point,
                                                       size_t k) const {
    auto start = std::chrono::high_resolution_clock::now();

    std::vector<SimplexID> results;

    SpatialIndexType active_type = (current_type_ == SpatialIndexType::AUTO) ?
                                   recommended_type_ : current_type_;

    if (active_type == SpatialIndexType::BVH && bvh_) {
        results = bvh_->nearest_neighbors(point, k);
    } else if (active_type == SpatialIndexType::OCTREE && octree_) {
        results = octree_->nearest_neighbors(point, k);
    }

    auto end = std::chrono::high_resolution_clock::now();
    if (profiling_enabled_) {
        update_metrics(end - start, 0);
        query_counts_[QueryType::NEAREST]++;
    }

    return results;
}

std::vector<SimplexID> SpatialIndex::query_frustum(const std::array<double, 16>& view_proj) const {
    auto start = std::chrono::high_resolution_clock::now();

    std::vector<SimplexID> results;

    SpatialIndexType active_type = (current_type_ == SpatialIndexType::AUTO) ?
                                   recommended_type_ : current_type_;

    if (active_type == SpatialIndexType::OCTREE && octree_) {
        results = octree_->query_frustum(view_proj);
    } else if (active_type == SpatialIndexType::BVH && bvh_) {
        // BVH doesn't support frustum, fallback to all simplices
        if (complex_) {
            for (const auto& [id, _] : complex_->get_all_simplices()) {
                results.push_back(id);
            }
        } else if (labeled_complex_) {
            for (const auto& [id, _] : labeled_complex_->get_all_simplices()) {
                results.push_back(id);
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    if (profiling_enabled_) {
        update_metrics(end - start, 0);
        query_counts_[QueryType::FRUSTUM]++;
    }

    return results;
}

void SpatialIndex::insert_simplex(SimplexID id) {
    // For simplicity, trigger rebuild
    if (adaptive_optimization_enabled_) {
        trigger_optimization();
    }
    rebuild();
}

void SpatialIndex::remove_simplex(SimplexID id) {
    rebuild();
}

void SpatialIndex::update_simplex(SimplexID id) {
    rebuild();
}

void SpatialIndex::enable_cache(bool enable) {
    cache_enabled_ = enable;
    if (!enable) {
        clear_cache();
    }
}

bool SpatialIndex::is_cache_enabled() const {
    return cache_enabled_;
}

void SpatialIndex::clear_cache() {
    query_cache_.clear();
}

void SpatialIndex::set_cache_size(size_t max_size) {
    max_cache_size_ = max_size;
    while (query_cache_.size() > max_size) {
        auto oldest = std::min_element(query_cache_.begin(), query_cache_.end(),
            [](const auto& a, const auto& b) {
                return a.second.timestamp < b.second.timestamp;
            });
        query_cache_.erase(oldest);
    }
}

size_t SpatialIndex::get_cache_size() const {
    return query_cache_.size();
}

double SpatialIndex::get_cache_hit_rate() const {
    if (metrics_.total_queries == 0) {
        return 0.0;
    }
    return metrics_.hit_rate;
}

void SpatialIndex::enable_profiling(bool enable) {
    profiling_enabled_ = enable;
    if (!enable) {
        reset_metrics();
    }
}

bool SpatialIndex::is_profiling_enabled() const {
    return profiling_enabled_;
}

SpatialIndexMetrics SpatialIndex::get_metrics() const {
    return metrics_;
}

void SpatialIndex::reset_metrics() {
    metrics_ = SpatialIndexMetrics();
    query_times_.clear();
    query_counts_.clear();
}

DataDistribution SpatialIndex::analyze_distribution() const {
    DataDistribution dist;

    size_t simplex_count = 0;
    std::array<double, 3> sum{0.0, 0.0, 0.0};
    std::array<double, 3> min{std::numeric_limits<double>::max(),
                            std::numeric_limits<double>::max(),
                            std::numeric_limits<double>::max()};
    std::array<double, 3> max{std::numeric_limits<double>::lowest(),
                            std::numeric_limits<double>::lowest(),
                            std::numeric_limits<double>::lowest()};

    if (complex_) {
        simplex_count = complex_->simplex_count();
        for (const auto& [id, simplex] : complex_->get_all_simplices()) {
            for (VertexID vid : simplex.vertices) {
                const auto& vertex = complex_->get_vertex(vid);
                sum[0] += vertex.x;
                sum[1] += vertex.y;
                sum[2] += vertex.z;
                min[0] = std::min(min[0], vertex.x);
                min[1] = std::min(min[1], vertex.y);
                min[2] = std::min(min[2], vertex.z);
                max[0] = std::max(max[0], vertex.x);
                max[1] = std::max(max[1], vertex.y);
                max[2] = std::max(max[2], vertex.z);
            }
        }
    } else if (labeled_complex_) {
        simplex_count = labeled_complex_->simplex_count();
        for (const auto& [id, simplex] : labeled_complex_->get_all_simplices()) {
            for (VertexID vid : simplex.vertices) {
                const auto& vertex = labeled_complex_->get_vertex(vid);
                sum[0] += vertex.x;
                sum[1] += vertex.y;
                sum[2] += vertex.z;
                min[0] = std::min(min[0], vertex.x);
                min[1] = std::min(min[1], vertex.y);
                min[2] = std::min(min[2], vertex.z);
                max[0] = std::max(max[0], vertex.x);
                max[1] = std::max(max[1], vertex.y);
                max[2] = std::max(max[2], vertex.z);
            }
        }
    }

    dist.simplex_count = simplex_count;
    if (simplex_count > 0) {
        dist.center_of_mass = {
            sum[0] / simplex_count,
            sum[1] / simplex_count,
            sum[2] / simplex_count
        };

        double extents[3] = {
            max[0] - min[0],
            max[1] - min[1],
            max[2] - min[2]
        };
        double max_extent = std::max({extents[0], extents[1], extents[2]});
        double min_extent = std::min({extents[0], extents[1], extents[2]});
        dist.extent_ratio = (min_extent > 0) ? max_extent / min_extent : 1.0;
    }

    return dist;
}

void SpatialIndex::print_analysis() const {
    DataDistribution dist = analyze_distribution();

    std::cout << "=== Spatial Index Analysis ===" << std::endl;
    std::cout << "Simplex count: " << dist.simplex_count << std::endl;
    std::cout << "Extent ratio: " << dist.extent_ratio << std::endl;
    std::cout << "Center of mass: (" << dist.center_of_mass[0] << ", "
              << dist.center_of_mass[1] << ", " << dist.center_of_mass[2] << ")" << std::endl;
    std::cout << "Recommended index type: "
              << (recommended_type_ == SpatialIndexType::BVH ? "BVH" : "Octree")
              << std::endl;
    std::cout << "=============================" << std::endl;
}

void SpatialIndex::enable_adaptive_optimization(bool enable) {
    adaptive_optimization_enabled_ = enable;
}

bool SpatialIndex::is_adaptive_optimization_enabled() const {
    return adaptive_optimization_enabled_;
}

void SpatialIndex::trigger_optimization() {
    analyze_data_distribution();

    SpatialIndexType new_type = recommended_type_;
    if (new_type != current_type_ && current_type_ != SpatialIndexType::AUTO) {
        std::cout << "Switching from "
                  << (current_type_ == SpatialIndexType::BVH ? "BVH" : "Octree")
                  << " to "
                  << (new_type == SpatialIndexType::BVH ? "BVH" : "Octree")
                  << " for better performance" << std::endl;
        rebuild(new_type);
    }
}

void SpatialIndex::identify_hotspots() {
    // Find frequently queried simplices
    std::unordered_map<SimplexID, size_t> simplex_access_count;

    for (const auto& [hash, entry] : query_cache_) {
        for (SimplexID id : entry.results) {
            simplex_access_count[id] += entry.access_count;
        }
    }

    // Store hotspots for future use (could add a member variable)
}

std::vector<SimplexID> SpatialIndex::get_hotspots(size_t top_n) const {
    std::unordered_map<SimplexID, size_t> simplex_access_count;

    for (const auto& [hash, entry] : query_cache_) {
        for (SimplexID id : entry.results) {
            simplex_access_count[id] += entry.access_count;
        }
    }

    std::vector<std::pair<size_t, SimplexID>> sorted;
    for (const auto& [id, count] : simplex_access_count) {
        sorted.push_back({count, id});
    }
    std::sort(sorted.begin(), sorted.end(), std::greater<>());

    std::vector<SimplexID> hotspots;
    for (size_t i = 0; i < std::min(top_n, sorted.size()); ++i) {
        hotspots.push_back(sorted[i].second);
    }

    return hotspots;
}

bool SpatialIndex::validate() const {
    if (current_type_ == SpatialIndexType::BVH || current_type_ == SpatialIndexType::AUTO) {
        if (bvh_ && !bvh_->validate()) {
            return false;
        }
    }
    if (current_type_ == SpatialIndexType::OCTREE || current_type_ == SpatialIndexType::AUTO) {
        if (octree_ && !octree_->validate()) {
            return false;
        }
    }
    return true;
}

void SpatialIndex::print_statistics() const {
    std::cout << "=== Spatial Index Statistics ===" << std::endl;
    std::cout << "Current type: "
              << (current_type_ == SpatialIndexType::BVH ? "BVH" :
                  current_type_ == SpatialIndexType::OCTREE ? "Octree" : "AUTO")
              << std::endl;
    std::cout << "Cache enabled: " << (cache_enabled_ ? "Yes" : "No") << std::endl;
    std::cout << "Cache size: " << query_cache_.size() << " / " << max_cache_size_ << std::endl;
    std::cout << "Profiling enabled: " << (profiling_enabled_ ? "Yes" : "No") << std::endl;
    std::cout << "Total queries: " << metrics_.total_queries << std::endl;
    std::cout << "Avg query time: " << metrics_.avg_query_time_ms << " ms" << std::endl;
    std::cout << "Max query time: " << metrics_.max_query_time_ms << " ms" << std::endl;
    std::cout << "Cache hit rate: " << (metrics_.hit_rate * 100.0) << "%" << std::endl;
    
    if (bvh_) {
        std::cout << "\nBVH nodes: " << bvh_->get_node_count() << std::endl;
        std::cout << "BVH depth: " << bvh_->get_depth() << std::endl;
        std::cout << "BVH leaves: " << bvh_->get_leaf_count() << std::endl;
    }
    
    if (octree_) {
        std::cout << "\nOctree nodes: " << octree_->get_node_count() << std::endl;
        std::cout << "Octree depth: " << octree_->get_depth() << std::endl;
        std::cout << "Octree leaves: " << octree_->get_leaf_count() << std::endl;
    }
    
    std::cout << "================================" << std::endl;
}

void SpatialIndex::analyze_data_distribution() {
    DataDistribution dist = analyze_distribution();

    // Recommend index type based on data characteristics
    if (dist.extent_ratio > 3.0 || dist.spatial_uniformity > 0.7) {
        // Highly elongated or uniform data -> Octree
        recommended_type_ = SpatialIndexType::OCTREE;
    } else {
        // Clustered or moderately distributed -> BVH
        recommended_type_ = SpatialIndexType::BVH;
    }
}

void SpatialIndex::switch_index(SpatialIndexType type) {
    if (type == SpatialIndexType::BVH) {
        if (complex_) {
            bvh_ = std::make_unique<BVH>(*complex_);
        } else if (labeled_complex_) {
            bvh_ = std::make_unique<BVH>(*labeled_complex_);
        }
        octree_.reset();
    } else if (type == SpatialIndexType::OCTREE) {
        if (complex_) {
            octree_ = std::make_unique<Octree>(*complex_);
        } else if (labeled_complex_) {
            octree_ = std::make_unique<Octree>(*labeled_complex_);
        }
        bvh_.reset();
    }
}

size_t SpatialIndex::hash_query(const std::array<double, 3>& point) const {
    // Simple hash for caching
    size_t h1 = std::hash<double>{}(point[0]);
    size_t h2 = std::hash<double>{}(point[1]);
    size_t h3 = std::hash<double>{}(point[2]);
    return h1 ^ (h2 << 1) ^ (h3 << 2);
}

void SpatialIndex::update_metrics(const std::chrono::duration<double>& elapsed,
                                 size_t nodes_visited) const {
    double elapsed_ms = elapsed.count() * 1000.0;
    
    metrics_.total_queries++;
    metrics_.avg_query_time_ms = 
        (metrics_.avg_query_time_ms * (metrics_.total_queries - 1) + elapsed_ms) /
        metrics_.total_queries;
    metrics_.max_query_time_ms = std::max(metrics_.max_query_time_ms, elapsed_ms);
    metrics_.avg_nodes_visited = 
        (metrics_.avg_nodes_visited * (metrics_.total_queries - 1) + nodes_visited) /
        metrics_.total_queries;
    
    query_times_.push_back(elapsed_ms);
    if (query_times_.size() > 1000) {
        query_times_.erase(query_times_.begin());
    }
}

bool SpatialIndex::should_optimize() const {
    // Check if performance is degrading
    if (query_times_.size() < 100) {
        return false;
    }

    double recent_avg = 0.0;
    for (size_t i = query_times_.size() - 100; i < query_times_.size(); ++i) {
        recent_avg += query_times_[i];
    }
    recent_avg /= 100.0;

    // If recent avg is 2x worse than overall avg, consider optimization
    return recent_avg > metrics_.avg_query_time_ms * 2.0;
}

} // namespace cebu
