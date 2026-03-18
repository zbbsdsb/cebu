# Spatial Indexing API

This document provides a comprehensive reference for Cebu's spatial indexing API, including BVH (Bounding Volume Hierarchy) and Octree implementations.

## Table of Contents

1. [Core Concepts](#core-concepts)
2. [Geometry Provider](#geometry-provider)
3. [BVH (Bounding Volume Hierarchy)](#bvh-bounding-volume-hierarchy)
4. [Octree](#octree)
5. [Spatial Queries](#spatial-queries)
6. [Advanced Features](#advanced-features)
7. [Performance Considerations](#performance-considerations)

---

## Core Concepts

### Spatial Index Types

| Class | Description | Use Case |
|-------|-------------|----------|
| `BVH` | Bounding Volume Hierarchy | General-purpose spatial indexing |
| `Octree` | 8-way space partitioning | Uniformly distributed data |

### Geometric Primitives

```cpp
// Point in 3D space
typedef struct Point {
    float x, y, z;
} Point;

// Axis-aligned bounding box
typedef struct AABB {
    Point min;
    Point max;
} AABB;

// Ray for ray casting
typedef struct Ray {
    Point origin;
    Point direction;
} Ray;
```

---

## Geometry Provider

The `GeometryProvider` interface is used to provide geometric data for spatial indexes.

### Class Definition

```cpp
class GeometryProvider {
public:
    virtual ~GeometryProvider() = default;
    
    // Get position for a simplex
    virtual Point get_position(SimplexID id) const = 0;
    
    // Get bounding box for a simplex
    virtual AABB get_bounds(SimplexID id) const;
};
```

### VertexGeometry

`VertexGeometry` is a concrete implementation of `GeometryProvider` that stores positions for vertices:

```cpp
class VertexGeometry : public GeometryProvider {
public:
    // Set position for a vertex
    void set_position(SimplexID vertex_id, const Point& position);
    
    // Get position for a vertex
    Point get_position(SimplexID id) const override;
    
    // Get bounding box for a simplex
    AABB get_bounds(SimplexID id) const override;
    
    // Clear all positions
    void clear();
};
```

---

## BVH (Bounding Volume Hierarchy)

### Class Definition

```cpp
class BVH {
public:
    // Build strategies
    enum class BuildStrategy {
        SAH,         // Surface Area Heuristic
        Median,      // Median split
        Midpoint,    // Midpoint split
        HLBVH,       // Hierarchical LBVH
        EqualCounts  // Equal counts split
    };
    
    // Constructors
    BVH(const SimplicialComplex& sc, const GeometryProvider& geometry);
    BVH(const SimplicialComplex& sc, const GeometryProvider& geometry, BuildStrategy strategy);
    BVH(const SimplicialComplex& sc, const GeometryProvider& geometry, BuildStrategy strategy, bool parallel);
    
    // Build/rebuild the BVH
    void build();
    void rebuild();
    
    // Incremental updates
    void insert(SimplexID id);
    void remove(SimplexID id);
    
    // Spatial queries
    SimplexID nearest_neighbor(const Point& point) const;
    std::vector<SimplexID> range_query(const Point& center, float radius) const;
    std::vector<SimplexID> aabb_query(const AABB& aabb) const;
    std::optional<SimplexID> ray_cast(const Ray& ray) const;
    
    // Statistics
    size_t node_count() const;
    size_t leaf_count() const;
    float average_depth() const;
    size_t max_depth() const;
};
```

### Constructor Parameters

| Parameter | Description | Default |
|-----------|-------------|---------|
| `sc` | Simplicial complex to index | N/A |
| `geometry` | Geometry provider for simplex positions | N/A |
| `strategy` | Build strategy | `BuildStrategy::SAH` |
| `parallel` | Enable parallel building | `false` |

### Methods

#### `nearest_neighbor`

```cpp
SimplexID nearest_neighbor(const Point& point) const;
```

Finds the simplex closest to the given point.

**Parameters:**
- `point`: Query point in 3D space

**Returns:**
- ID of the nearest simplex

#### `range_query`

```cpp
std::vector<SimplexID> range_query(const Point& center, float radius) const;
```

Finds all simplices within a sphere defined by center and radius.

**Parameters:**
- `center`: Center of the query sphere
- `radius`: Radius of the query sphere

**Returns:**
- Vector of simplex IDs within the range

#### `aabb_query`

```cpp
std::vector<SimplexID> aabb_query(const AABB& aabb) const;
```

Finds all simplices intersecting the given axis-aligned bounding box.

**Parameters:**
- `aabb`: Axis-aligned bounding box

**Returns:**
- Vector of simplex IDs intersecting the AABB

#### `ray_cast`

```cpp
std::optional<SimplexID> ray_cast(const Ray& ray) const;
```

Finds the first simplex intersected by the given ray.

**Parameters:**
- `ray`: Ray for casting

**Returns:**
- Optional simplex ID of the first hit, or `std::nullopt` if no hit

---

## Octree

### Class Definition

```cpp
class Octree {
public:
    // Constructors
    Octree(const SimplicialComplex& sc, const GeometryProvider& geometry);
    Octree(const SimplicialComplex& sc, const GeometryProvider& geometry, int max_depth);
    Octree(const SimplicialComplex& sc, const GeometryProvider& geometry, int max_depth, bool parallel);
    
    // Build/rebuild the octree
    void build();
    void rebuild();
    
    // Incremental updates
    void insert(SimplexID id);
    void remove(SimplexID id);
    
    // Spatial queries
    SimplexID nearest_neighbor(const Point& point) const;
    std::vector<SimplexID> range_query(const Point& center, float radius) const;
    std::vector<SimplexID> aabb_query(const AABB& aabb) const;
    std::optional<SimplexID> ray_cast(const Ray& ray) const;
    
    // Statistics
    size_t node_count() const;
    size_t leaf_count() const;
    float average_depth() const;
    size_t max_depth() const;
};
```

### Constructor Parameters

| Parameter | Description | Default |
|-----------|-------------|---------|
| `sc` | Simplicial complex to index | N/A |
| `geometry` | Geometry provider for simplex positions | N/A |
| `max_depth` | Maximum depth of the octree | `10` |
| `parallel` | Enable parallel building | `false` |

### Methods

The Octree class provides the same query methods as BVH:
- `nearest_neighbor`
- `range_query`
- `aabb_query`
- `ray_cast`

Refer to the BVH section for method details.

---

## Spatial Queries

### Query Performance

| Query Type | BVH Time Complexity | Octree Time Complexity |
|------------|---------------------|------------------------|
| Nearest Neighbor | O(log n) | O(log n) |
| Range Query | O(k + log n) | O(k + log n) |
| AABB Query | O(k + log n) | O(k + log n) |
| Ray Cast | O(log n + k) | O(log n + k) |

Where:
- `n` is the number of simplices
- `k` is the number of results

### Query Examples

#### Nearest Neighbor

```cpp
// Create BVH
cebu::BVH bvh(sc, vg);

// Query nearest neighbor
cebu::Point query_point = {0.5, 0.5, 0.0};
cebu::SimplexID nearest = bvh.nearest_neighbor(query_point);
std::cout << "Nearest simplex: " << nearest << std::endl;
```

#### Range Query

```cpp
// Create Octree
cebu::Octree octree(sc, vg);

// Query range
cebu::Point center = {0.0, 0.0, 0.0};
float radius = 1.0;
auto results = octree.range_query(center, radius);
std::cout << "Found " << results.size() << " simplices in range" << std::endl;
```

#### AABB Query

```cpp
// Create BVH
cebu::BVH bvh(sc, vg);

// Query AABB
cebu::AABB aabb;
aabb.min = {0.0, 0.0, 0.0};
aabb.max = {1.0, 1.0, 1.0};
auto results = bvh.aabb_query(aabb);
std::cout << "Found " << results.size() << " simplices in AABB" << std::endl;
```

#### Ray Cast

```cpp
// Create Octree
cebu::Octree octree(sc, vg);

// Cast ray
cebu::Ray ray;
ray.origin = {0.0, 0.0, -1.0};
ray.direction = {0.0, 0.0, 1.0};
auto hit = octree.ray_cast(ray);
if (hit) {
    std::cout << "Ray hit simplex: " << *hit << std::endl;
} else {
    std::cout << "Ray missed" << std::endl;
}
```

---

## Advanced Features

### Custom Geometry Providers

You can create custom geometry providers for non-standard vertex data:

```cpp
class CustomGeometryProvider : public cebu::GeometryProvider {
public:
    // Constructor
    CustomGeometryProvider(const std::unordered_map<cebu::SimplexID, cebu::Point>& positions)
        : positions_(positions) {}
    
    // Get position for a simplex
    cebu::Point get_position(cebu::SimplexID id) const override {
        auto it = positions_.find(id);
        if (it != positions_.end()) {
            return it->second;
        }
        return {0.0, 0.0, 0.0}; // Default position
    }
    
private:
    std::unordered_map<cebu::SimplexID, cebu::Point> positions_;
};

// Usage
std::unordered_map<cebu::SimplexID, cebu::Point> custom_positions;
custom_positions[0] = {0.0, 0.0, 0.0};
custom_positions[1] = {1.0, 0.0, 0.0};
custom_positions[2] = {0.0, 1.0, 0.0};

CustomGeometryProvider custom_geometry(custom_positions);
cebu::BVH bvh(sc, custom_geometry);
```

### Parallel Building

Enable parallel building for faster index construction:

```cpp
// BVH with parallel building
cebu::BVH bvh_parallel(
    sc, 
    vg, 
    cebu::BVH::BuildStrategy::SAH,
    true  // Enable parallel build
);

// Octree with parallel building
cebu::Octree octree_parallel(
    sc, 
    vg, 
    10,  // Max depth
    true  // Enable parallel build
);
```

### Incremental Updates

For dynamic scenes, you can use incremental updates:

```cpp
// Add a new vertex
auto v4 = sc.add_vertex();
vg.set_position(v4, {1.0, 1.0, 1.0});

// Incrementally update BVH
bvh.insert(v4);

// Add a triangle
auto tri = sc.add_triangle(v1, v2, v4);

// Incrementally update BVH
bvh.insert(tri);
```

---

## Performance Considerations

### BVH vs Octree

| Feature | BVH | Octree |
|---------|-----|--------|
| Build Time | Slower (especially SAH) | Faster |
| Query Time | Faster for complex geometries | Faster for uniformly distributed data |
| Memory Usage | Higher | Lower |
| Best For | Irregularly distributed data | Uniformly distributed data |
| Scalability | Good for large datasets | Good for point clouds |

### Build Strategy Performance

| BVH Build Strategy | Build Time | Query Performance | Memory Usage |
|--------------------|------------|-------------------|-------------|
| `SAH` | Slowest | Best | High |
| `Median` | Medium | Good | Medium |
| `Midpoint` | Fastest | Fair | Low |
| `HLBVH` | Fast | Very Good | Medium |
| `EqualCounts` | Medium | Good | Medium |

### Optimization Tips

1. **Choose the right index type**: Use BVH for general cases, Octree for uniformly distributed data
2. **Select appropriate build strategy**: Use SAH for static scenes, Median for dynamic scenes
3. **Enable parallel building**: Use parallel building for large datasets
4. **Use incremental updates**: For dynamic scenes, use incremental updates instead of full rebuilds
5. **Tune octree depth**: Adjust max_depth based on dataset size (typically 8-12)
6. **Cache geometry data**: Precompute and cache geometry data for faster queries
7. **Use appropriate data structures**: For large point clouds, consider using Octree
8. **Benchmark different configurations**: Test different settings to find the optimal configuration for your use case

---

## Code Example

### Complete Spatial Indexing Example

```cpp
#include "cebu/simplicial_complex.h"
#include "cebu/vertex_geometry.h"
#include "cebu/bvh.h"
#include "cebu/octree.h"
#include <iostream>

int main() {
    using namespace cebu;
    
    // Create simplicial complex
    SimplicialComplex sc;
    
    // Create vertex geometry
    VertexGeometry vg;
    
    // Add vertices with positions
    auto v0 = sc.add_vertex();
    auto v1 = sc.add_vertex();
    auto v2 = sc.add_vertex();
    auto v3 = sc.add_vertex();
    
    vg.set_position(v0, {0.0, 0.0, 0.0});
    vg.set_position(v1, {1.0, 0.0, 0.0});
    vg.set_position(v2, {0.0, 1.0, 0.0});
    vg.set_position(v3, {1.0, 1.0, 1.0});
    
    // Add triangles
    sc.add_triangle(v0, v1, v2);
    sc.add_triangle(v1, v2, v3);
    
    // Build BVH
    BVH bvh(sc, vg, BVH::BuildStrategy::SAH, true);
    
    // Build Octree
    Octree octree(sc, vg, 8, true);
    
    // Test nearest neighbor
    Point query_point = {0.5, 0.5, 0.5};
    auto bvh_nearest = bvh.nearest_neighbor(query_point);
    auto octree_nearest = octree.nearest_neighbor(query_point);
    
    std::cout << "Nearest neighbor (BVH): " << bvh_nearest << std::endl;
    std::cout << "Nearest neighbor (Octree): " << octree_nearest << std::endl;
    
    // Test range query
    Point center = {0.0, 0.0, 0.0};
    float radius = 2.0;
    
    auto bvh_range = bvh.range_query(center, radius);
    auto octree_range = octree.range_query(center, radius);
    
    std::cout << "\nRange query results:" << std::endl;
    std::cout << "BVH found " << bvh_range.size() << " simplices" << std::endl;
    std::cout << "Octree found " << octree_range.size() << " simplices" << std::endl;
    
    // Test ray cast
    Ray ray;
    ray.origin = {0.5, 0.5, -1.0};
    ray.direction = {0.0, 0.0, 1.0};
    
    auto bvh_ray = bvh.ray_cast(ray);
    auto octree_ray = octree.ray_cast(ray);
    
    std::cout << "\nRay cast results:" << std::endl;
    if (bvh_ray) {
        std::cout << "BVH ray hit simplex: " << *bvh_ray << std::endl;
    } else {
        std::cout << "BVH ray missed" << std::endl;
    }
    
    if (octree_ray) {
        std::cout << "Octree ray hit simplex: " << *octree_ray << std::endl;
    } else {
        std::cout << "Octree ray missed" << std::endl;
    }
    
    return 0;
}
```

---

**Cebu Version**: 0.8.0
**Last Updated**: 2026-03-18