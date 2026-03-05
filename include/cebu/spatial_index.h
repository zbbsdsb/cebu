#ifndef CEBU_SPATIAL_INDEX_H
#define CEBU_SPATIAL_INDEX_H

#include "bvh.h"
#include "octree.h"
#include "simplicial_complex.h"
#include "simplicial_complex_labeled.h"
#include <memory>
#include <vector>
#include <array>
#include <chrono>
#include <unordered_map>

namespace cebu {

/**
 * @brief Spatial index type
 */
enum class SpatialIndexType {
    BVH,      // Bounding Volume Hierarchy
    OCTREE,   // Octree
    AUTO      // Automatically select based on data
};

/**
 * @brief Spatial query type
 */
enum class QueryType {
    POINT,        // Point containment
    RANGE,        // Axis-aligned bounding box
    SPHERE,       // Sphere
    NEAREST,      // Nearest neighbor
    FRUSTUM       // Frustum culling
};

/**
 * @brief Performance metrics for spatial index
 */
struct SpatialIndexMetrics {
    double build_time_ms;
    double avg_query_time_ms;
    double max_query_time_ms;
    size_t total_queries;
    size_t avg_nodes_visited;
    size_t memory_usage_bytes;
    double hit_rate;  // Cache hit rate for cached results

    SpatialIndexMetrics()
        : build_time_ms(0.0)
        , avg_query_time_ms(0.0)
        , max_query_time_ms(0.0)
        , total_queries(0)
        , avg_nodes_visited(0)
        , memory_usage_bytes(0)
        , hit_rate(0.0) {}
};

/**
 * @brief Data distribution characteristics
 */
struct DataDistribution {
    size_t simplex_count;
    double spatial_uniformity;  // 0.0 = clustered, 1.0 = uniform
    double density_variance;
    std::array<double, 3> center_of_mass;
    double extent_ratio;  // ratio of largest to smallest dimension

    DataDistribution()
        : simplex_count(0)
        , spatial_uniformity(0.5)
        , density_variance(0.0)
        , center_of_mass{0.0, 0.0, 0.0}
        , extent_ratio(1.0) {}
};

/**
 * @brief Query cache entry
 */
struct QueryCacheEntry {
    std::vector<SimplexID> results;
    std::chrono::system_clock::time_point timestamp;
    size_t access_count;

    QueryCacheEntry() : access_count(0) {}
};

/**
 * @brief High-performance unified spatial index
 * 
 * Automatically selects and manages the best spatial index strategy
 * based on data distribution and query patterns.
 */
class SpatialIndex {
public:
    SpatialIndex();
    explicit SpatialIndex(const SimplicialComplex& complex);
    explicit SpatialIndex(const SimplicialComplexLabeled<>& complex);
    
    ~SpatialIndex();

    // Build and rebuild
    void build(const SimplicialComplex& complex);
    void build(const SimplicialComplexLabeled<>& complex);
    void rebuild();
    void rebuild(SpatialIndexType type);

    // Index type management
    void set_index_type(SpatialIndexType type);
    SpatialIndexType get_index_type() const;
    SpatialIndexType get_recommended_index_type() const;

    // Query operations
    bool contains(const std::array<double, 3>& point) const;
    std::vector<SimplexID> query_point(const std::array<double, 3>& point) const;
    std::vector<SimplexID> query_range(const std::array<double, 3>& min,
                                       const std::array<double, 3>& max) const;
    std::vector<SimplexID> query_sphere(const std::array<double, 3>& center, double radius) const;
    std::vector<SimplexID> nearest_neighbors(const std::array<double, 3>& point, size_t k) const;
    std::vector<SimplexID> query_frustum(const std::array<double, 16>& view_proj) const;

    // Update operations
    void insert_simplex(SimplexID id);
    void remove_simplex(SimplexID id);
    void update_simplex(SimplexID id);

    // Cache management
    void enable_cache(bool enable);
    bool is_cache_enabled() const;
    void clear_cache();
    void set_cache_size(size_t max_size);
    size_t get_cache_size() const;
    double get_cache_hit_rate() const;

    // Performance monitoring
    void enable_profiling(bool enable);
    bool is_profiling_enabled() const;
    SpatialIndexMetrics get_metrics() const;
    void reset_metrics();

    // Data analysis
    DataDistribution analyze_distribution() const;
    void print_analysis() const;

    // Adaptive optimization
    void enable_adaptive_optimization(bool enable);
    bool is_adaptive_optimization_enabled() const;
    void trigger_optimization();
    
    // Hotspot management
    void identify_hotspots();
    std::vector<SimplexID> get_hotspots(size_t top_n) const;

    // Validation
    bool validate() const;
    void print_statistics() const;

private:
    std::unique_ptr<BVH> bvh_;
    std::unique_ptr<Octree> octree_;
    const SimplicialComplex* complex_;
    const SimplicialComplexLabeled<>* labeled_complex_;
    SpatialIndexType current_type_;
    SpatialIndexType recommended_type_;

    // Cache
    bool cache_enabled_;
    size_t max_cache_size_;
    std::unordered_map<size_t, QueryCacheEntry> query_cache_;
    
    // Profiling
    bool profiling_enabled_;
    mutable SpatialIndexMetrics metrics_;
    mutable std::vector<double> query_times_;
    
    // Adaptive optimization
    bool adaptive_optimization_enabled_;
    std::unordered_map<QueryType, size_t> query_counts_;

    // Internal methods
    void analyze_data_distribution();
    void switch_index(SpatialIndexType type);
    size_t hash_query(const std::array<double, 3>& point) const;
    void update_metrics(const std::chrono::duration<double>& elapsed, size_t nodes_visited) const;
    bool should_optimize() const;
};

} // namespace cebu

#endif // CEBU_SPATIAL_INDEX_H
