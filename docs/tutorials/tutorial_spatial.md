# Spatial Indexing Tutorial

This tutorial will guide you through using Cebu's spatial indexing features, including BVH (Bounding Volume Hierarchy) and Octree for accelerated spatial queries.

## Table of Contents

1. [Introduction](#introduction)
2. [Setting Up](#setting-up)
3. [BVH (Bounding Volume Hierarchy)](#bvh-bounding-volume-hierarchy)
4. [Octree](#octree)
5. [Spatial Queries](#spatial-queries)
6. [Performance Considerations](#performance-considerations)
7. [Advanced Features](#advanced-features)
8. [Complete Example](#complete-example)

---

## Introduction

Spatial indexing is a technique used to accelerate spatial queries by organizing geometric data in a hierarchical structure. Cebu provides two main spatial index implementations:

- **BVH (Bounding Volume Hierarchy)**: A general-purpose spatial index that works well for most scenarios
- **Octree**: A 8-way space partitioning index that excels for uniformly distributed data

These indexes can significantly speed up spatial queries like nearest neighbor search, range queries, and collision detection.

---

## Setting Up

To use spatial indexing in Cebu, you need to include the appropriate headers and have spatial indexing enabled in your build.

### Required Headers

```cpp
#include "cebu/simplicial_complex.h"
#include "cebu/vertex_geometry.h"
#include "cebu/bvh.h"      // For BVH
#include "cebu/octree.h"    // For Octree
```

### Basic Setup

```cpp
#include "cebu/simplicial_complex.h"
#include "cebu/vertex_geometry.h"
#include "cebu/bvh.h"
#include <iostream>

int main() {
    // Create a simplicial complex
    cebu::SimplicialComplex sc;
    
    // Create vertex geometry to store positions
    cebu::VertexGeometry vg;
    
    // Add vertices with positions
    auto v0 = sc.add_vertex();
    auto v1 = sc.add_vertex();
    auto v2 = sc.add_vertex();
    
    vg.set_position(v0, {0.0, 0.0, 0.0});
    vg.set_position(v1, {1.0, 0.0, 0.0});
    vg.set_position(v2, {0.0, 1.0, 0.0});
    
    // Add a triangle
    sc.add_triangle(v0, v1, v2);
    
    // Continue with spatial indexing setup...
    
    return 0;
}
```

---

## BVH (Bounding Volume Hierarchy)

### Creating a BVH

```cpp
// Build BVH from simplicial complex and vertex geometry
cebu::BVH bvh(sc, vg);

// Optional: Specify build strategy
cebu::BVH bvh_with_strategy(
    sc, 
    vg, 
    cebu::BVHBuildStrategy::SAH  // Surface Area Heuristic (default)
);
```

### BVH Build Strategies

| Strategy | Description | Use Case |
|----------|-------------|----------|
| `SAH` | Surface Area Heuristic - optimal for most cases | General use |
| `Median` | Median split - fast build, good for balanced data | Real-time applications |
| `Midpoint` | Midpoint split - very fast build | Rapid prototyping |
| `HLBVH` | Hierarchical LBVH - fast build for large datasets | Large point clouds |
| `EqualCounts` | Equal counts split - balanced tree | Uniformly distributed data |

### Updating BVH

```cpp
// After modifying the complex, rebuild the BVH
sc.add_vertex();
vg.set_position(3, {1.0, 1.0, 0.0});
sc.add_triangle(1, 2, 3);

bvh.rebuild();
```

---

## Octree

### Creating an Octree

```cpp
// Build Octree from simplicial complex and vertex geometry
cebu::Octree octree(sc, vg);

// Optional: Specify maximum depth
cebu::Octree octree_with_depth(
    sc, 
    vg, 
    8  // Maximum depth (default: 10)
);
```

### Octree Parameters

| Parameter | Description | Default |
|-----------|-------------|---------|
| `max_depth` | Maximum depth of the octree | 10 |
| `min_primitives` | Minimum primitives per leaf node | 1 |

### Updating Octree

```cpp
// After modifying the complex, rebuild the octree
octree.rebuild();
```

---

## Spatial Queries

### Nearest Neighbor Search

```cpp
// Define a query point
cebu::Point query_point = {0.5, 0.5, 0.0};

// BVH nearest neighbor
auto bvh_nearest = bvh.nearest_neighbor(query_point);
std::cout << "BVH nearest simplex: " << bvh_nearest << std::endl;

// Octree nearest neighbor
auto octree_nearest = octree.nearest_neighbor(query_point);
std::cout << "Octree nearest simplex: " << octree_nearest << std::endl;
```

### Range Queries

```cpp
// Define a query sphere (center and radius)
cebu::Point center = {0.0, 0.0, 0.0};
float radius = 1.5;

// BVH range query
auto bvh_results = bvh.range_query(center, radius);
std::cout << "BVH found " << bvh_results.size() << " simplices in range" << std::endl;

// Octree range query
auto octree_results = octree.range_query(center, radius);
std::cout << "Octree found " << octree_results.size() << " simplices in range" << std::endl;
```

### AABB (Axis-Aligned Bounding Box) Queries

```cpp
// Define AABB
cebu::AABB aabb;
aabb.min = {0.0, 0.0, 0.0};
aabb.max = {1.0, 1.0, 0.0};

// BVH AABB query
auto bvh_aabb_results = bvh.aabb_query(aabb);
std::cout << "BVH found " << bvh_aabb_results.size() << " simplices in AABB" << std::endl;

// Octree AABB query
auto octree_aabb_results = octree.aabb_query(aabb);
std::cout << "Octree found " << octree_aabb_results.size() << " simplices in AABB" << std::endl;
```

### Ray Casting

```cpp
// Define a ray
cebu::Ray ray;
ray.origin = {0.0, -1.0, 0.0};
ray.direction = {0.0, 1.0, 0.0};

// BVH ray cast
auto bvh_ray_hit = bvh.ray_cast(ray);
if (bvh_ray_hit.has_value()) {
    std::cout << "BVH ray hit simplex: " << bvh_ray_hit.value() << std::endl;
}

// Octree ray cast
auto octree_ray_hit = octree.ray_cast(ray);
if (octree_ray_hit.has_value()) {
    std::cout << "Octree ray hit simplex: " << octree_ray_hit.value() << std::endl;
}
```

---

## Performance Considerations

### When to Use BVH vs Octree

| Index Type | Best For | Worst For |
|------------|----------|-----------|
| **BVH** | Irregularly distributed data, complex geometries, general use | None - versatile |
| **Octree** | Uniformly distributed data, point clouds, voxel-based data | Highly irregular data |

### Build Time vs Query Time

- **BVH**: Slower build time, faster query time (especially with SAH strategy)
- **Octree**: Faster build time, slightly slower query time for complex geometries

### Memory Usage

- **BVH**: Higher memory usage due to bounding volumes
- **Octree**: Lower memory usage, especially for deep trees

---

## Advanced Features

### Incremental Updates

For dynamic scenes, you can use incremental updates instead of full rebuilds:

```cpp
// Add a new vertex and update the BVH incrementally
auto v4 = sc.add_vertex();
vg.set_position(v4, {0.5, 0.5, 1.0});
sc.add_triangle(v0, v2, v4);

bvh.insert(v4);  // Incrementally insert new vertex
bvh.insert(4);    // Incrementally insert new triangle
```

### Custom Geometry Providers

You can create custom geometry providers for non-standard vertex data:

```cpp
class CustomGeometryProvider : public cebu::GeometryProvider {
public:
    cebu::Point get_position(cebu::SimplexID id) const override {
        // Custom logic to get position for simplex
        return {0.0, 0.0, 0.0};
    }
};

CustomGeometryProvider custom_geometry;
cebu::BVH bvh_custom(sc, custom_geometry);
```

### Parallel Building

Enable parallel building for faster index construction:

```cpp
// Enable parallel BVH build
cebu::BVH bvh_parallel(
    sc, 
    vg, 
    cebu::BVHBuildStrategy::SAH,
    true  // Enable parallel build
);

// Enable parallel Octree build
cebu::Octree octree_parallel(
    sc, 
    vg, 
    10,  // Max depth
    true  // Enable parallel build
);
```

---

## Complete Example

```cpp
#include "cebu/simplicial_complex.h"
#include "cebu/vertex_geometry.h"
#include "cebu/bvh.h"
#include "cebu/octree.h"
#include <iostream>

int main() {
    using namespace cebu;
    
    // Create a simplicial complex
    SimplicialComplex sc;
    
    // Create vertex geometry
    VertexGeometry vg;
    
    // Add vertices with positions (a simple cube)
    auto v0 = sc.add_vertex();
    auto v1 = sc.add_vertex();
    auto v2 = sc.add_vertex();
    auto v3 = sc.add_vertex();
    auto v4 = sc.add_vertex();
    auto v5 = sc.add_vertex();
    auto v6 = sc.add_vertex();
    auto v7 = sc.add_vertex();
    
    vg.set_position(v0, {0.0, 0.0, 0.0});
    vg.set_position(v1, {1.0, 0.0, 0.0});
    vg.set_position(v2, {1.0, 1.0, 0.0});
    vg.set_position(v3, {0.0, 1.0, 0.0});
    vg.set_position(v4, {0.0, 0.0, 1.0});
    vg.set_position(v5, {1.0, 0.0, 1.0});
    vg.set_position(v6, {1.0, 1.0, 1.0});
    vg.set_position(v7, {0.0, 1.0, 1.0});
    
    // Add cube faces
    sc.add_triangle(v0, v1, v2);
    sc.add_triangle(v0, v2, v3);
    sc.add_triangle(v1, v5, v6);
    sc.add_triangle(v1, v6, v2);
    sc.add_triangle(v5, v4, v7);
    sc.add_triangle(v5, v7, v6);
    sc.add_triangle(v4, v0, v3);
    sc.add_triangle(v4, v3, v7);
    sc.add_triangle(v3, v2, v6);
    sc.add_triangle(v3, v6, v7);
    sc.add_triangle(v4, v5, v1);
    sc.add_triangle(v4, v1, v0);
    
    // Build BVH
    BVH bvh(sc, vg, BVHBuildStrategy::SAH, true);
    
    // Build Octree
    Octree octree(sc, vg, 8, true);
    
    // Test nearest neighbor
    Point query_point = {0.5, 0.5, 1.5};
    
    auto bvh_nearest = bvh.nearest_neighbor(query_point);
    auto octree_nearest = octree.nearest_neighbor(query_point);
    
    std::cout << "Query point: (0.5, 0.5, 1.5)" << std::endl;
    std::cout << "BVH nearest simplex: " << bvh_nearest << std::endl;
    std::cout << "Octree nearest simplex: " << octree_nearest << std::endl;
    
    // Test range query
    Point center = {0.5, 0.5, 0.5};
    float radius = 1.0;
    
    auto bvh_range = bvh.range_query(center, radius);
    auto octree_range = octree.range_query(center, radius);
    
    std::cout << "\nRange query (center: (0.5, 0.5, 0.5), radius: 1.0)" << std::endl;
    std::cout << "BVH found " << bvh_range.size() << " simplices" << std::endl;
    std::cout << "Octree found " << octree_range.size() << " simplices" << std::endl;
    
    // Test AABB query
    AABB aabb;
    aabb.min = {0.0, 0.0, 0.0};
    aabb.max = {1.0, 1.0, 0.5};
    
    auto bvh_aabb = bvh.aabb_query(aabb);
    auto octree_aabb = octree.aabb_query(aabb);
    
    std::cout << "\nAABB query (0.0-1.0, 0.0-1.0, 0.0-0.5)" << std::endl;
    std::cout << "BVH found " << bvh_aabb.size() << " simplices" << std::endl;
    std::cout << "Octree found " << octree_aabb.size() << " simplices" << std::endl;
    
    // Test ray casting
    Ray ray;
    ray.origin = {0.5, 0.5, -1.0};
    ray.direction = {0.0, 0.0, 1.0};
    
    auto bvh_ray = bvh.ray_cast(ray);
    auto octree_ray = octree.ray_cast(ray);
    
    std::cout << "\nRay cast from (0.5, 0.5, -1.0) along (0,0,1)" << std::endl;
    if (bvh_ray.has_value()) {
        std::cout << "BVH ray hit simplex: " << bvh_ray.value() << std::endl;
    } else {
        std::cout << "BVH ray missed" << std::endl;
    }
    
    if (octree_ray.has_value()) {
        std::cout << "Octree ray hit simplex: " << octree_ray.value() << std::endl;
    } else {
        std::cout << "Octree ray missed" << std::endl;
    }
    
    return 0;
}
```

### Expected Output

```
Query point: (0.5, 0.5, 1.5)
BVH nearest simplex: 10
Octree nearest simplex: 10

Range query (center: (0.5, 0.5, 0.5), radius: 1.0)
BVH found 12 simplices
Octree found 12 simplices

AABB query (0.0-1.0, 0.0-1.0, 0.0-0.5)
BVH found 4 simplices
Octree found 4 simplices

Ray cast from (0.5, 0.5, -1.0) along (0,0,1)
BVH ray hit simplex: 1
Octree ray hit simplex: 1
```

---

## Next Steps

Now that you've learned the basics of spatial indexing in Cebu, you can:

- Explore the [Spatial Indexing API documentation](../api/api_spatial.md) for more details
- Try using spatial indexing with labeled complexes
- Experiment with different build strategies for optimal performance
- Integrate spatial indexing into your own projects

For more advanced topics, see the [Performance Guide](../utilities/performance.md) for benchmarking and optimization tips.

---

**Cebu Version**: 0.8.0
**Last Updated**: 2026-03-18