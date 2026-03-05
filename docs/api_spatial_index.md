# Spatial Indexing API

The `spatial_index` namespace provides high-performance spatial indexing for simplicial complexes.

---

## Overview

Cebu provides three spatial indexing strategies:

1. **BVH (Bounding Volume Hierarchy)** - Optimal for clustered or irregularly distributed data
2. **Octree** - Uniform space subdivision, ideal for regular grids or uniform distributions
3. **SpatialIndex** - Hybrid manager that automatically selects the best strategy

---

## BVH (Bounding Volume Hierarchy)

### BoundingBox

3D axis-aligned bounding box with intersection and containment tests.

```cpp
struct BoundingBox {
    std::array<double, 3> min;
    std::array<double, 3> max;
    SimplexID simplex_id;

    // Expand to include point
    void expand(const std::array<double, 3>& point);
    
    // Merge with another box
    void merge(const BoundingBox& other);
    
    // Surface area (for SAH optimization)
    double surface_area() const;
    
    // Containment and intersection tests
    bool contains(const std::array<double, 3>& point) const;
    bool intersects(const BoundingBox& other) const;
    
    // Center point
    std::array<double, 3> center() const;
};
```

### BVHBuildStrategy

Build strategy for BVH construction.

```cpp
enum class BVHBuildStrategy {
    MIDPOINT,   // Split at midpoint
    MEDIAN,     // Split at median (balanced tree)
    SAH,        // Surface Area Heuristic (optimal)
    HILBERT     // Hilbert curve ordering
};
```

### BVHQueryResult

Query result with performance metrics.

```cpp
struct BVHQueryResult {
    std::vector<SimplexID> simplex_ids;
    size_t nodes_visited;
    double query_time_ms;
};
```

### BVH Class

High-performance Bounding Volume Hierarchy.

```cpp
class BVH {
public:
    BVH();
    explicit BVH(const SimplicialComplex& complex);
    explicit BVH(const SimplicialComplexLabeled<>& complex);
    
    // Build BVH
    void build(const SimplicialComplex& complex);
    void rebuild();
    void rebuild(BVHBuildStrategy strategy);
    
    // Point queries
    bool contains(const std::array<double, 3>& point) const;
    std::vector<SimplexID> query_point(const std::array<double, 3>& point) const;
    
    // Range queries
    BVHQueryResult query_range(const BoundingBox& range) const;
    BVHQueryResult query_sphere(const std::array<double, 3>& center, double radius) const;
    
    // Nearest neighbor
    std::vector<SimplexID> nearest_neighbors(const std::array<double, 3>& point, size_t k) const;
    
    // Ray intersection
    std::vector<SimplexID> ray_intersect(const std::array<double, 3>& origin,
                                         const std::array<double, 3>& direction) const;
    
    // Update operations
    void update_simplex(SimplexID id, const std::array<double, 3>& new_position);
    void remove_simplex(SimplexID id);
    
    // Statistics
    size_t get_node_count() const;
    size_t get_depth() const;
    size_t get_leaf_count() const;
    size_t get_simplex_count() const;
    double get_average_leaf_size() const;
    
    // Configuration
    void set_build_strategy(BVHBuildStrategy strategy);
    void set_max_leaf_size(size_t size);
    void set_max_depth(size_t depth);
    
    // Validation
    bool validate() const;
    void print_tree() const;
};
```

#### Example: Using BVH

```cpp
#include "cebu/bvh.h"

using namespace cebu;

SimplicialComplex complex;
// ... build complex ...

// Build BVH with SAH strategy
BVH bvh(complex);
bvh.set_build_strategy(BVHBuildStrategy::SAH);
bvh.rebuild();

// Query
std::array<double, 3> point = {0.5, 0.3, 0.2};
auto results = bvh.query_point(point);

// Nearest neighbors
auto neighbors = bvh.nearest_neighbors(point, 10);

// Statistics
std::cout << "Nodes: " << bvh.get_node_count() << "\n";
std::cout << "Depth: " << bvh.get_depth() << "\n";
std::cout << "Avg leaf size: " << bvh.get_average_leaf_size() << "\n";
```

---

## Octree

### OctreeNode

Octree node with 8 children.

```cpp
struct OctreeNode {
    std::array<double, 3> center;
    double extent;
    std::vector<SimplexID> simplices;
    std::array<std::unique_ptr<OctreeNode>, 8> children;
    bool is_leaf;

    void get_bounds(std::array<double, 3>& min, std::array<double, 3>& max) const;
    bool contains(const std::array<double, 3>& point) const;
};
```

### OctreeQueryResult

Query result with performance metrics.

```cpp
struct OctreeQueryResult {
    std::vector<SimplexID> simplex_ids;
    size_t nodes_visited;
    size_t depth;
};
```

### Octree Class

Octree spatial index with LOD support.

```cpp
class Octree {
public:
    Octree();
    explicit Octree(const SimplicialComplex& complex);
    explicit Octree(const SimplicialComplexLabeled<>& complex);
    
    // Build octree
    void build(const SimplicialComplex& complex);
    void rebuild();
    
    // Point queries
    bool contains(const std::array<double, 3>& point) const;
    std::vector<SimplexID> query_point(const std::array<double, 3>& point) const;
    
    // Range queries
    OctreeQueryResult query_range(const std::array<double, 3>& min,
                                 const std::array<double, 3>& max) const;
    OctreeQueryResult query_sphere(const std::array<double, 3>& center, double radius) const;
    
    // Nearest neighbor
    std::vector<SimplexID> nearest_neighbors(const std::array<double, 3>& point, size_t k) const;
    
    // Frustum culling
    std::vector<SimplexID> query_frustum(const std::array<double, 16>& view_proj) const;
    
    // Update operations
    void insert_simplex(SimplexID id);
    void remove_simplex(SimplexID id);
    void update_simplex(SimplexID id);
    
    // LOD operations
    void set_lod_level(int level);
    int get_lod_level() const;
    std::vector<SimplexID> get_lod_simplices(int lod_level) const;
    
    // Statistics
    size_t get_node_count() const;
    size_t get_depth() const;
    size_t get_leaf_count() const;
    size_t get_simplex_count() const;
    size_t get_max_simplices_per_node() const;
    
    // Configuration
    void set_max_simplices_per_node(size_t max);
    void set_max_depth(size_t depth);
    
    // Validation
    bool validate() const;
    void print_tree() const;
};
```

#### Example: Using Octree

```cpp
#include "cebu/octree.h"

using namespace cebu;

SimplicialComplex complex;
// ... build complex ...

// Build octree
Octree octree(complex);

// LOD queries
octree.set_lod_level(2);
auto simplified = octree.get_lod_simplices(2);

// Frustum culling
std::array<double, 16> view_proj = {
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0
};
auto visible = octree.query_frustum(view_proj);

// Statistics
std::cout << "Nodes: " << octree.get_node_count() << "\n";
std::cout << "Depth: " << octree.get_depth() << "\n";
```

---

## SpatialIndex (Hybrid Index)

### SpatialIndexType

Spatial index type.

```cpp
enum class SpatialIndexType {
    BVH,      // Bounding Volume Hierarchy
    OCTREE,   // Octree
    AUTO      // Automatically select based on data
};
```

### QueryType

Query type for statistics.

```cpp
enum class QueryType {
    POINT,        // Point containment
    RANGE,        // Axis-aligned bounding box
    SPHERE,       // Sphere
    NEAREST,      // Nearest neighbor
    FRUSTUM       // Frustum culling
};
```

### SpatialIndexMetrics

Performance metrics for spatial index.

```cpp
struct SpatialIndexMetrics {
    double build_time_ms;
    double avg_query_time_ms;
    double max_query_time_ms;
    size_t total_queries;
    size_t avg_nodes_visited;
    size_t memory_usage_bytes;
    double hit_rate;
};
```

### DataDistribution

Data distribution characteristics.

```cpp
struct DataDistribution {
    size_t simplex_count;
    double spatial_uniformity;
    double density_variance;
    std::array<double, 3> center_of_mass;
    double extent_ratio;
};
```

### SpatialIndex Class

High-performance unified spatial index with automatic optimization.

```cpp
class SpatialIndex {
public:
    SpatialIndex();
    explicit SpatialIndex(const SimplicialComplex& complex);
    explicit SpatialIndex(const SimplicialComplexLabeled<>& complex);
    
    // Build and rebuild
    void build(const SimplicialComplex& complex);
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
};
```

#### Example: Using SpatialIndex

```cpp
#include "cebu/spatial_index.h"

using namespace cebu;

SimplicialComplex complex;
// ... build complex ...

// Create spatial index (auto-selects best strategy)
SpatialIndex index(complex);

// Enable caching and profiling
index.enable_cache(true);
index.set_cache_size(1000);
index.enable_profiling(true);

// Query operations
std::array<double, 3> point = {0.5, 0.3, 0.2};
auto results = index.query_point(point);

auto range_results = index.query_range({0.0, 0.0, 0.0}, {1.0, 1.0, 1.0});
auto sphere_results = index.query_sphere(point, 1.0);
auto neighbors = index.nearest_neighbors(point, 10);

// Get performance metrics
auto metrics = index.get_metrics();
std::cout << "Avg query time: " << metrics.avg_query_time_ms << " ms\n";
std::cout << "Cache hit rate: " << (metrics.hit_rate * 100) << "%\n";

// Adaptive optimization
index.enable_adaptive_optimization(true);
index.trigger_optimization();

// Hotspot analysis
auto hotspots = index.get_hotspots(10);
std::cout << "Top 10 hotspots:\n";
for (SimplexID id : hotspots) {
    std::cout << "  " << id << "\n";
}

// Data analysis
auto distribution = index.analyze_distribution();
std::cout << "Spatial uniformity: " << distribution.spatial_uniformity << "\n";
std::cout << "Recommended index: "
          << (index.get_recommended_index_type() == SpatialIndexType::BVH ? "BVH" : "Octree")
          << "\n";
```

---

## Performance Guidelines

### Choosing the Right Index

| Data Characteristics | Recommended Index |
|---------------------|-------------------|
| Clustered data | BVH |
| Irregular distribution | BVH |
| Regular grid | Octree |
| Uniform distribution | Octree |
| Unknown/Auto | SpatialIndex (AUTO) |

### Optimization Tips

1. **Enable caching** for repeated queries:
   ```cpp
   index.enable_cache(true);
   index.set_cache_size(1000);
   ```

2. **Use profiling** to monitor performance:
   ```cpp
   index.enable_profiling(true);
   auto metrics = index.get_metrics();
   ```

3. **Enable adaptive optimization** for dynamic workloads:
   ```cpp
   index.enable_adaptive_optimization(true);
   ```

4. **Choose the right BVH strategy**:
   - `MIDPOINT`: Fast build, good for general use
   - `MEDIAN`: Balanced tree, good for querying
   - `SAH`: Optimal, slower build, best queries
   - `HILBERT`: Good cache locality

5. **Use LOD for rendering**:
   ```cpp
   octree.set_lod_level(2);
   auto simplified = octree.get_lod_simplices(2);
   ```

### Performance Expectations

| Operation | Expected Performance |
|-----------|---------------------|
| Point query | < 1μs |
| Range query | < 100μs |
| kNN (k=10) | < 1ms |
| Build (100k) | < 100ms |
| Cache hit rate | > 90% |

---

## Complete Example

```cpp
#include "cebu/simplicial_complex.h"
#include "cebu/spatial_index.h"
#include <iostream>

using namespace cebu;

int main() {
    // Create simplicial complex
    SimplicialComplex complex;
    
    // Add vertices (grid)
    size_t grid_size = 50;
    for (size_t x = 0; x < grid_size; ++x) {
        for (size_t y = 0; y < grid_size; ++y) {
            for (size_t z = 0; z < grid_size; ++z) {
                size_t id = x * grid_size * grid_size + y * grid_size + z;
                complex.add_vertex(id, static_cast<double>(x),
                                   static_cast<double>(y),
                                   static_cast<double>(z));
            }
        }
    }
    
    // Create simplices
    size_t simplex_id = 0;
    for (size_t x = 0; x < grid_size - 1; ++x) {
        for (size_t y = 0; y < grid_size - 1; ++y) {
            for (size_t z = 0; z < grid_size - 1; ++z) {
                complex.add_simplex(simplex_id++, {
                    x * grid_size * grid_size + y * grid_size + z,
                    (x + 1) * grid_size * grid_size + y * grid_size + z,
                    x * grid_size * grid_size + (y + 1) * grid_size + z,
                    x * grid_size * grid_size + y * grid_size + (z + 1)
                });
            }
        }
    }
    
    std::cout << "Created " << complex.simplex_count() << " simplices\n";
    
    // Build spatial index
    SpatialIndex index(complex);
    index.enable_cache(true);
    index.enable_profiling(true);
    
    // Analyze data
    auto distribution = index.analyze_distribution();
    std::cout << "Recommended index: "
              << (index.get_recommended_index_type() == SpatialIndexType::BVH ? "BVH" : "Octree")
              << "\n";
    
    // Perform queries
    std::array<double, 3> center = {25.0, 25.0, 25.0};
    
    // Point query
    auto point_results = index.query_point(center);
    std::cout << "Point query found " << point_results.size() << " simplices\n";
    
    // Sphere query
    auto sphere_results = index.query_sphere(center, 5.0);
    std::cout << "Sphere query found " << sphere_results.size() << " simplices\n";
    
    // Nearest neighbors
    auto neighbors = index.nearest_neighbors(center, 10);
    std::cout << "Found " << neighbors.size() << " nearest neighbors\n";
    
    // Get metrics
    auto metrics = index.get_metrics();
    std::cout << "\nPerformance Metrics:\n";
    std::cout << "  Build time: " << metrics.build_time_ms << " ms\n";
    std::cout << "  Avg query time: " << metrics.avg_query_time_ms << " ms\n";
    std::cout << "  Max query time: " << metrics.max_query_time_ms << " ms\n";
    std::cout << "  Total queries: " << metrics.total_queries << "\n";
    std::cout << "  Cache hit rate: " << (metrics.hit_rate * 100) << "%\n";
    
    return 0;
}
```

---

## See Also

- [Main API Documentation](api.md)
- [Architecture Documentation](architecture.md)
- [Advanced Serialization API](api_advanced_serialization.md)
