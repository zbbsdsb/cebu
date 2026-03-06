# Topology Operations Guide

This guide covers topological operations available in Cebu for analyzing and manipulating simplicial complexes.

## Table of Contents

1. [Euler Characteristic](#euler-characteristic)
2. [Manifold Checking](#manifold-checking)
3. [Boundary Computation](#boundary-computation)
4. [Connected Components](#connected-components)
5. [Vertex Gluing](#vertex-gluing)
6. [Use Cases](#use-cases)
7. [API Reference](#api-reference)

---

## Euler Characteristic

The **Euler characteristic** is a fundamental topological invariant:

```
χ = V - E + F - T + ...
```

Where V, E, F, T are counts of vertices, edges, faces, tetrahedra, etc.

### Computing Euler Characteristic

```cpp
#include "cebu/topology_operations.h"

cebu::SimplicialComplex complex;

// Build complex...

int euler = cebu::TopologyOperations::compute_euler_characteristic(complex);
std::cout << "Euler characteristic: " << euler << std::endl;
```

### Interpreting Results

| Shape | Euler Characteristic | Description |
|-------|-------------------|-------------|
| Sphere | 2 | Closed surface |
| Torus | 0 | One hole |
| Double Torus | -2 | Two holes |
| Disk | 1 | Has boundary |
| Möbius Strip | 0 | Non-orientable |
| Projective Plane | 1 | Non-orientable |

### Example: Verify Sphere

```cpp
// Create a triangulated sphere
cebu::SimplicialComplex complex;

// ... add triangles forming sphere ...

int euler = cebu::TopologyOperations::compute_euler_characteristic(complex);
if (euler == 2) {
    std::cout << "Complex is topologically a sphere" << std::endl;
}
```

### Example: Count Holes

```cpp
// For surfaces, genus g = (2 - χ) / 2
int euler = cebu::TopologyOperations::compute_euler_characteristic(complex);
int genus = (2 - euler) / 2;

std::cout << "Complex has " << genus << " holes" << std::endl;
```

---

## Manifold Checking

A **manifold** is a space that locally looks like Euclidean space.

### Checking Manifold Property

```cpp
#include "cebu/topology_operations.h"

cebu::SimplicialComplex complex;

// Build complex...

bool is_manifold = cebu::TopologyOperations::is_manifold(complex);
if (is_manifold) {
    std::cout << "Complex is a manifold" << std::endl;
} else {
    std::cout << "Complex is NOT a manifold" << std::endl;
}
```

### What Makes a Complex a Manifold?

For a 2D simplicial complex:
1. Each edge is shared by at most 2 triangles
2. Each vertex is contained in a single "star" (no pinching)

### Non-Manifold Examples

#### Pinched Vertex
```cpp
// Create a pinched vertex (two sheets meeting at a point)
auto v0 = complex.add_simplex({});
auto v1 = complex.add_simplex({});
auto v2 = complex.add_simplex({});
auto v3 = complex.add_simplex({});  // Pinch point

// Two triangles meeting only at v3
complex.add_simplex({v0, v1, v3});
complex.add_simplex({v2, v1, v3});

// This is NOT a manifold
bool is_manifold = cebu::TopologyOperations::is_manifold(complex);
// is_manifold == false
```

#### Edge Shared by > 2 Triangles
```cpp
// Edge shared by 3 triangles
auto v0 = complex.add_simplex({});
auto v1 = complex.add_simplex({});
auto v2 = complex.add_simplex({});
auto v3 = complex.add_simplex({});

complex.add_simplex({v0, v1, v2});  // Edge v0-v1
complex.add_simplex({v0, v1, v3});  // Edge v0-v1 again
complex.add_simplex({v0, v1, v4});  // Edge v0-v1 third time

// NOT a manifold (edge in > 2 triangles)
```

### Edge-Based Manifold Checking

```cpp
// Get edges that violate manifold property
auto non_manifold_edges =
    cebu::TopologyOperations::get_non_manifold_edges(complex);

for (auto edge_id : non_manifold_edges) {
    std::cout << "Non-manifold edge: " << edge_id << std::endl;
}
```

---

## Boundary Computation

The **boundary** of a complex consists of simplices not faces of any higher-dimensional simplex.

### Computing Boundary

```cpp
#include "cebu/topology_operations.h"

cebu::SimplicialComplex complex;

// Build complex...

auto boundary = cebu::TopologyOperations::compute_boundary(complex);

std::cout << "Boundary has " << boundary.size() << " simplices" << std::endl;
```

### Example: Disk Boundary

```cpp
// Create a triangulated disk (sphere with a hole)
cebu::SimplicialComplex complex;

// ... add triangles forming disk with hole ...

auto boundary = cebu::TopologyOperations::compute_boundary(complex);

// Boundary should form a closed loop
for (auto simplex_id : boundary) {
    std::cout << "Boundary simplex: " << simplex_id << std::endl;
}
```

### Checking if Complex is Closed

```cpp
auto boundary = cebu::TopologyOperations::compute_boundary(complex);
bool is_closed = boundary.empty();

if (is_closed) {
    std::cout << "Complex has no boundary (closed surface)" << std::endl;
} else {
    std::cout << "Complex has boundary with " << boundary.size()
              << " simplices" << std::endl;
}
```

### Example: Verify Sphere vs Disk

```cpp
// Sphere: No boundary
cebu::SimplicialComplex sphere;
// ... build sphere ...
auto sphere_boundary = cebu::TopologyOperations::compute_boundary(sphere);
bool sphere_closed = sphere_boundary.empty();  // true

// Disk: Has boundary
cebu::SimplicialComplex disk;
// ... build disk with hole ...
auto disk_boundary = cebu::TopologyOperations::compute_boundary(disk);
bool disk_closed = disk_boundary.empty();  // false
```

---

## Connected Components

Connected components are maximal connected subcomplexes.

### Finding Connected Components

```cpp
#include "cebu/topology_operations.h"

cebu::SimplicialComplex complex;

// Build complex with multiple disconnected parts...

auto components = cebu::TopologyOperations::get_connected_components(complex);

std::cout << "Complex has " << components.size()
          << " connected components" << std::endl;

for (size_t i = 0; i < components.size(); ++i) {
    std::cout << "Component " << i << " has "
              << components[i].size() << " simplices" << std::endl;
}
```

### Example: Two Separate Objects

```cpp
// Create two separate triangles
cebu::SimplicialComplex complex;

// Triangle 1
auto v0 = complex.add_simplex({});
auto v1 = complex.add_simplex({});
auto v2 = complex.add_simplex({});
complex.add_simplex({v0, v1, v2});

// Triangle 2 (disconnected)
auto v3 = complex.add_simplex({});
auto v4 = complex.add_simplex({});
auto v5 = complex.add_simplex({});
complex.add_simplex({v3, v4, v5});

// Should have 2 components
auto components = cebu::TopologyOperations::get_connected_components(complex);
assert(components.size() == 2);
```

### Extracting a Component

```cpp
// Extract the largest connected component
auto components = cebu::TopologyOperations::get_connected_components(complex);

// Find largest
auto largest = std::max_element(
    components.begin(),
    components.end(),
    [](const auto& a, const auto& b) {
        return a.size() < b.size();
    }
);

// Create new complex with only largest component
cebu::SimplicialComplex largest_component;
for (auto simplex_id : *largest) {
    const auto& simplex = complex.get_simplex(simplex_id);
    largest_component.add_simplex(simplex.vertices());
}
```

### Checking Connectivity

```cpp
bool is_connected = cebu::TopologyOperations::is_connected(complex);
if (is_connected) {
    std::cout << "Complex is connected" << std::endl;
} else {
    std::cout << "Complex is disconnected" << std::endl;
}
```

---

## Vertex Gluing

Gluing vertices identifies them as the same topological point.

### Basic Vertex Gluing

```cpp
#include "cebu/topology_operations.h"

cebu::SimplicialComplex complex;

// Add vertices
auto v0 = complex.add_simplex({});
auto v1 = complex.add_simplex({});
auto v2 = complex.add_simplex({});

// Add edges
auto e01 = complex.add_simplex({v0, v1});
auto e12 = complex.add_simplex({v1, v2});

// Glue v0 and v2 (will merge v0 into v2)
cebu::TopologyOperations::glue_vertices(complex, v0, v2);

// Now e01 has been updated to {v2, v1}
// v0 no longer exists
```

### Creating a Circle from Line Segment

```cpp
// Create a line segment
cebu::SimplicialComplex complex;

auto v0 = complex.add_simplex({});
auto v1 = complex.add_simplex({});
auto v2 = complex.add_simplex({});

complex.add_simplex({v0, v1});
complex.add_simplex({v1, v2});

// Glue endpoints to create circle
cebu::TopologyOperations::glue_vertices(complex, v0, v2);

// Now topologically a circle
```

### Creating Projective Plane

```cpp
// Create a square mesh and glue edges with twist
cebu::SimplicialComplex complex;

// ... create triangulated square ...

// Glue opposite edges with twist
// Top edge (v0-v1-v2-...) glued to bottom edge (vN-vN-1-vN-2-...)
for (size_t i = 0; i < top_edge.size(); ++i) {
    cebu::TopologyOperations::glue_vertices(
        complex,
        top_edge[i],
        bottom_edge[bottom_edge.size() - 1 - i]
    );
}

// Result: Projective plane (non-orientable)
```

---

## Use Cases

### 1. Mesh Validation

Check if mesh is valid before processing:

```cpp
#include "cebu/topology_operations.h"

void validate_mesh(const cebu::SimplicialComplex& mesh) {
    // Check if manifold
    if (!cebu::TopologyOperations::is_manifold(mesh)) {
        throw std::runtime_error("Mesh is not a manifold");
    }

    // Check if closed
    auto boundary = cebu::TopologyOperations::compute_boundary(mesh);
    if (!boundary.empty()) {
        std::cout << "Warning: Mesh has boundary" << std::endl;
    }

    // Check if connected
    if (!cebu::TopologyOperations::is_connected(mesh)) {
        throw std::runtime_error("Mesh is disconnected");
    }

    std::cout << "Mesh validation passed" << std::endl;
}
```

### 2. Mesh Repair

Fix non-manifold edges:

```cpp
void repair_non_manifold_edges(cebu::SimplicialComplex& mesh) {
    auto bad_edges =
        cebu::TopologyOperations::get_non_manifold_edges(mesh);

    for (auto edge_id : bad_edges) {
        // Strategy 1: Remove edge
        mesh.remove_simplex(edge_id);

        // Strategy 2: Duplicate edge (more complex)
        // ...
    }

    std::cout << "Repaired " << bad_edges.size()
              << " non-manifold edges" << std::endl;
}
```

### 3. Topological Analysis

Analyze complex properties:

```cpp
void analyze_topology(const cebu::SimplicialComplex& complex) {
    // Euler characteristic
    int euler = cebu::TopologyOperations::compute_euler_characteristic(complex);
    std::cout << "Euler characteristic: " << euler << std::endl;

    // Genus (for surfaces)
    int genus = (2 - euler) / 2;
    std::cout << "Genus: " << genus << std::endl;

    // Connected components
    auto components =
        cebu::TopologyOperations::get_connected_components(complex);
    std::cout << "Connected components: " << components.size() << std::endl;

    // Boundary
    auto boundary = cebu::TopologyOperations::compute_boundary(complex);
    std::cout << "Boundary simplices: " << boundary.size() << std::endl;

    // Manifold check
    bool is_manifold = cebu::TopologyOperations::is_manifold(complex);
    std::cout << "Is manifold: " << (is_manifold ? "yes" : "no") << std::endl;
}
```

### 4. Shape Classification

Classify surface based on topology:

```cpp
enum class SurfaceType {
    Sphere,
    Torus,
    DoubleTorus,
    Disk,
    Unknown
};

SurfaceType classify_surface(const cebu::SimplicialComplex& complex) {
    // Compute boundary
    auto boundary = cebu::TopologyOperations::compute_boundary(complex);

    // If has boundary, it's a disk-like shape
    if (!boundary.empty()) {
        return SurfaceType::Disk;
    }

    // Otherwise, classify by Euler characteristic
    int euler = cebu::TopologyOperations::compute_euler_characteristic(complex);

    if (euler == 2) {
        return SurfaceType::Sphere;
    } else if (euler == 0) {
        // Could be torus or Klein bottle
        return SurfaceType::Torus;  // Simplified
    } else if (euler == -2) {
        return SurfaceType::DoubleTorus;
    } else {
        return SurfaceType::Unknown;
    }
}
```

### 5. Mesh Segmentation

Separate mesh into connected components:

```cpp
std::vector<cebu::SimplicialComplex> segment_mesh(
    const cebu::SimplicialComplex& mesh
) {
    auto components =
        cebu::TopologyOperations::get_connected_components(mesh);

    std::vector<cebu::SimplicialComplex> segments;

    for (const auto& component : components) {
        cebu::SimplicialComplex segment;
        for (auto simplex_id : component) {
            const auto& simplex = mesh.get_simplex(simplex_id);
            segment.add_simplex(simplex.vertices());
        }
        segments.push_back(std::move(segment));
    }

    return segments;
}
```

---

## API Reference

### TopologyOperations

```cpp
namespace cebu::TopologyOperations {

    /// Compute Euler characteristic
    static int compute_euler_characteristic(
        const SimplicialComplex& complex
    );

    /// Check if complex is a manifold
    static bool is_manifold(
        const SimplicialComplex& complex
    );

    /// Get non-manifold edges
    static std::vector<SimplexID> get_non_manifold_edges(
        const SimplicialComplex& complex
    );

    /// Compute boundary of complex
    static std::vector<SimplexID> compute_boundary(
        const SimplicialComplex& complex
    );

    /// Get connected components
    static std::vector<std::vector<SimplexID>> get_connected_components(
        const SimplicialComplex& complex
    );

    /// Check if complex is connected
    static bool is_connected(
        const SimplicialComplex& complex
    );

    /// Glue two vertices together
    static VertexID glue_vertices(
        SimplicialComplex& complex,
        VertexID v1,
        VertexID v2
    );

    /// Check if simplex A is face of simplex B
    static bool is_face_of(
        const Simplex& a,
        const Simplex& b
    );

    /// Compute boundary of a simplex
    static std::vector<SimplexID> compute_simplex_boundary(
        const SimplicialComplex& complex,
        SimplexID simplex_id
    );
}
```

---

## Performance Considerations

### Complexity Analysis

| Operation | Time Complexity | Space Complexity |
|-----------|---------------|------------------|
| Euler Characteristic | O(n) | O(1) |
| Manifold Check | O(n) | O(1) |
| Boundary Computation | O(n²) | O(n) |
| Connected Components | O(n) | O(n) |
| Vertex Gluing | O(k) | O(1) |

Where n is number of simplices, k is number of simplices containing glued vertex.

### Optimization Tips

1. **Cache Results**: For static complexes, cache topological invariants
2. **Incremental Updates**: Use incremental algorithms for dynamic complexes
3. **Parallel Processing**: Connected components can be computed in parallel

---

## Summary

Topology operations provide powerful analysis tools:

- **Euler Characteristic**: Topological invariant
- **Manifold Checking**: Validate mesh quality
- **Boundary Computation**: Identify edges and holes
- **Connected Components**: Analyze mesh connectivity
- **Vertex Gluing**: Create quotient spaces

Use these operations to validate, analyze, and manipulate simplicial complexes.
