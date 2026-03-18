# Equivalence Classes and Non-Hausdorff Topology

This guide explains how to use Cebu's equivalence class system to create non-Hausdorff topological spaces by gluing simplices together.

## Table of Contents

1. [Concepts](#concepts)
2. [Equivalence Classes](#equivalence-classes)
3. [Non-Hausdorff Simplicial Complex](#non-hausdorff-simplicial-complex)
4. [Common Use Cases](#common-use-cases)
5. [API Reference](#api-reference)

---

## Concepts

### What is Non-Hausdorff Topology?

In standard (Hausdorff) topologies, any two distinct points have disjoint neighborhoods. In **non-Hausdorff topology**, this property doesn't hold - points can be "glued" together while maintaining separate identities.

### What is Simplex Gluing?

**Gluing** identifies two or more simplices as topologically equivalent:
- They share the same topological position
- They can have different labels or attributes
- They appear as one entity in topology queries

### Applications

- **Identifying Boundaries**: Glue edges of a square to create a cylinder or Möbius strip
- **Symmetry Operations**: Identify equivalent vertices in symmetric structures
- **Topological Identifications**: Create quotient spaces
- **Mesh Merging**: Combine meshes along common boundaries

---

## Equivalence Classes

### EquivalenceClassManager

Manages glued simplices using the Union-Find (Disjoint Set Union) data structure with path compression.

#### Creating an Equivalence Class Manager

```cpp
#include "cebu/equivalence_classes.h"

cebu::EquivalenceClassManager manager;
```

#### Adding Simplices

```cpp
// Add simplices to the manager
manager.add_simplex(vertex_id_0);
manager.add_simplex(vertex_id_1);
manager.add_simplex(vertex_id_2);
```

#### Gluing Simplices

```cpp
// Glue vertex_id_1 and vertex_id_2 together
manager.glue(vertex_id_1, vertex_id_2);

// Now they are in the same equivalence class
auto class1 = manager.find_class(vertex_id_1);
auto class2 = manager.find_class(vertex_id_2);
// class1 == class2 (true)
```

#### Querying Equivalence

```cpp
// Check if two simplices are glued together
bool are_glued = manager.are_equivalent(vertex_id_1, vertex_id_2);

// Get all members of an equivalence class
auto members = manager.get_class_members(vertex_id_1);
for (auto id : members) {
    std::cout << "Member: " << id << std::endl;
}

// Get the representative simplex
auto representative = manager.find_representative(vertex_id_1);
```

#### Ungluing

```cpp
// Separate a simplex from its equivalence class
manager.unglue(vertex_id_1);

// Now vertex_id_1 is in its own class
bool still_glued = manager.are_equivalent(vertex_id_1, vertex_id_2);
// still_glued == false
```

---

## Non-Hausdorff Simplicial Complex

The `SimplicialComplexNonHausdorff` extends the standard simplicial complex with gluing capabilities.

### Basic Usage

```cpp
#include "cebu/simplicial_complex_non_hausdorff.h"

// Create a non-Hausdorff complex
cebu::SimplicialComplexNonHausdorff complex;
```

### Creating a Cylinder (Gluing Square Edges)

```cpp
#include "cebu/simplicial_complex_non_hausdorff.h"
#include <iostream>

int main() {
    cebu::SimplicialComplexNonHausdorff complex;

    // Create a square as two triangles
    //   v1 ---- v2
    //   | \  / |
    //   |  v4  |
    //   | /  \ |
    //   v0 ---- v3

    auto v0 = complex.add_simplex({});
    auto v1 = complex.add_simplex({});
    auto v2 = complex.add_simplex({});
    auto v3 = complex.add_simplex({});

    // Add triangles
    auto tri1 = complex.add_simplex({v0, v1, v2});
    auto tri2 = complex.add_simplex({v0, v2, v3});

    // Glue v1 to v3 (create cylinder by identifying left and right edges)
    complex.glue_simplices(v1, v3);

    // Check equivalence
    bool glued = complex.are_glued(v1, v3);
    std::cout << "v1 and v3 are glued: " << glued << std::endl;

    // Get all glued vertices
    auto glued_group = complex.get_glued_simplices(v1);
    std::cout << "Glued group size: " << glued_group.size() << std::endl;

    return 0;
}
```

### Creating a Möbius Strip

```cpp
// Create a Möbius strip by gluing edges with a twist
cebu::SimplicialComplexNonHausdorff complex;

// Create a strip of triangles (simplified)
// ... add vertices and triangles ...

// Glue with twist: (v0, v1) glued to (vn, vn-1)
complex.glue_simplices(v0, vn);
complex.glue_simplices(v1, vn_minus_1);
```

### Gluing with Labeled Complexes

```cpp
#include "cebu/simplicial_complex_non_hausdorff.h"
#include "cebu/label.h"

using ComplexNH = cebu::SimplicialComplexNonHausdorffLabeled<float>;

ComplexNH complex;

// Add vertices with different labels
auto v0 = complex.add_simplex({}, 10.0f);
auto v1 = complex.add_simplex({}, 20.0f);

// Glue them (they remain topologically equivalent but keep different labels)
complex.glue_simplices(v0, v1);

// Query both labels
float label0 = complex.get_label(v0);  // 10.0f
float label1 = complex.get_label(v1);  // 20.0f

// But they're in the same equivalence class
bool glued = complex.are_glued(v0, v1);  // true
```

---

## Common Use Cases

### 1. Topological Identification

Create quotient spaces by identifying points:

```cpp
// Identify boundary of a disk to a single point
cebu::SimplicialComplexNonHausdorff complex;

// ... create disk of triangles ...

// Glue all boundary vertices to a single point
auto boundary = /* get boundary vertices */;
auto representative = boundary[0];

for (size_t i = 1; i < boundary.size(); ++i) {
    complex.glue_simplices(representative, boundary[i]);
}

// Result: Sphere (disk + point = sphere)
```

### 2. Symmetric Meshes

Identify symmetric vertices:

```cpp
// Create symmetric cube
cebu::SimplicialComplexNonHausdorff complex;

// ... create cube with 8 vertices ...

// Glue opposite vertices (identify through center)
complex.glue_simplices(v0, v6);
complex.glue_simplices(v1, v7);
complex.glue_simplices(v2, v4);
complex.glue_simplices(v3, v5);

// Result: Projective plane-like structure
```

### 3. Mesh Merging

Combine two meshes along a common boundary:

```cpp
cebu::SimplicialComplexNonHausdorff complex;

// Add mesh 1
// ... add vertices and triangles for mesh 1 ...

// Add mesh 2
// ... add vertices and triangles for mesh 2 ...

// Glue boundary vertices
auto boundary1 = /* get boundary of mesh 1 */;
auto boundary2 = /* get boundary of mesh 2 */;

assert(boundary1.size() == boundary2.size());

for (size_t i = 0; i < boundary1.size(); ++i) {
    complex.glue_simplices(boundary1[i], boundary2[i]);
}

// Result: Single merged mesh
```

### 4. Torus Construction

Create a torus by gluing opposite edges of a square:

```cpp
cebu::SimplicialComplexNonHausdorff complex;

// Create square mesh
// ... create triangulated square ...

// Glue top and bottom edges
for (size_t i = 0; i < top_edge.size(); ++i) {
    complex.glue_simplices(top_edge[i], bottom_edge[i]);
}

// Glue left and right edges
for (size_t i = 0; i < left_edge.size(); ++i) {
    complex.glue_simplices(left_edge[i], right_edge[i]);
}

// Result: Torus
```

---

## API Reference

### EquivalenceClassManager

```cpp
class EquivalenceClassManager {
public:
    // Add a simplex (initially in its own class)
    void add_simplex(SimplexID simplex_id);

    // Remove a simplex from all classes
    void remove_simplex(SimplexID simplex_id);

    // Glue two simplices (union their classes)
    void glue(SimplexID simplex_a, SimplexID simplex_b);

    // Unglue a simplex from its class
    void unglue(SimplexID simplex_id);

    // Check if two simplices are equivalent
    bool are_equivalent(SimplexID simplex_a, SimplexID simplex_b) const;

    // Find the representative of a simplex's class
    SimplexID find_representative(SimplexID simplex_id) const;

    // Get all members of an equivalence class
    std::vector<SimplexID> get_class_members(SimplexID simplex_id) const;

    // Get the equivalence class for a simplex
    const EquivalenceClass& find_class(SimplexID simplex_id) const;

    // Get all equivalence classes
    const std::unordered_map<SimplexID, EquivalenceClass>& get_all_classes() const;

    // Get number of equivalence classes
    size_t class_count() const;
};
```

### SimplicialComplexNonHausdorff

```cpp
class SimplicialComplexNonHausdorff : public SimplicialComplex {
public:
    // Glue two simplices together
    void glue_simplices(SimplexID simplex_a, SimplexID simplex_b);

    // Unglue a simplex
    void unglue_simplex(SimplexID simplex_id);

    // Check if two simplices are glued
    bool are_glued(SimplexID simplex_a, SimplexID simplex_b) const;

    // Get all simplices glued to the given one
    std::vector<SimplexID> get_glued_simplices(SimplexID simplex_id) const;

    // Get the equivalence class manager
    const EquivalenceClassManager& get_equivalence_manager() const;
};
```

### Labeled Non-Hausdorff Complex

```cpp
template<typename LabelType>
class SimplicialComplexNonHausdorffLabeled
    : public SimplicialComplexLabeled<LabelType> {
public:
    // Glue two simplices together
    void glue_simplices(SimplexID simplex_a, SimplexID simplex_b);

    // Unglue a simplex
    void unglue_simplex(SimplexID simplex_id);

    // Check if two simplices are glued
    bool are_glued(SimplexID simplex_a, SimplexID simplex_b) const;

    // Get all simplices glued to the given one
    std::vector<SimplexID> get_glued_simplices(SimplexID simplex_id) const;
};
```

---

## Best Practices

### 1. Glue Before Adding Relationships

Always glue simplices before adding faces that depend on them:

```cpp
// Good
complex.add_simplex({});  // v0
complex.add_simplex({});  // v1
complex.glue_simplices(v0, v1);
complex.add_simplex({v0, v1});  // Edge works correctly

// Bad
auto v0 = complex.add_simplex({});
auto v1 = complex.add_simplex({});
auto e01 = complex.add_simplex({v0, v1});
complex.glue_simplices(v0, v1);  // May cause issues
```

### 2. Use Representatives for Queries

When querying glued simplices, use the representative:

```cpp
auto rep = complex.find_representative(simplex_id);
auto neighbors = complex.get_neighbors(rep);  // Query representative
```

### 3. Handle Label Conflicts

Be aware that glued simplices can have different labels:

```cpp
complex.add_simplex({}, 10.0f);  // v0 with label 10.0
complex.add_simplex({}, 20.0f);  // v1 with label 20.0
complex.glue_simplices(v0, v1);

// They're glued but have different labels
// Decide how to handle this in your application
```

### 4. Consider Topology vs Geometry

Gluing affects topology, not geometry. For spatial queries with glued simplices, consider:

```cpp
// Topologically glued, but geometrically separate
complex.glue_simplices(v0, v1);

// Spatial queries need special handling
// Use equivalence classes to group geometric positions
```

---

## Performance Considerations

### Union-Find Complexity

- **add_simplex**: O(1)
- **glue**: O(α(n)) ≈ O(1) with path compression
- **are_equivalent**: O(α(n)) ≈ O(1)
- **get_class_members**: O(k) where k is class size

### Memory Overhead

Each equivalence class stores:
- Representative simplex ID
- Set of member simplex IDs

**Approximate overhead**: ~32 bytes per simplex

---

## Limitations

### 1. No Automatic Geometric Gluing

The system doesn't automatically adjust vertex positions. You must handle geometry manually:

```cpp
// Gluing doesn't move vertices
complex.glue_simplices(v0, v1);

// You may need to manually align geometry
if (needs_alignment) {
    align_geometry(v0, v1);
}
```

### 2. Cascade Deletion

Removing a simplex unglues it from its class but doesn't affect others:

```cpp
complex.remove_simplex(simplex_id);
// Other members of the equivalence class remain
```

### 3. Label Independence

Glued simplices maintain independent labels. If you need shared labels, handle explicitly:

```cpp
// After gluing, synchronize labels if needed
complex.glue_simplices(v0, v1);
auto shared_label = compute_shared_label(v0, v1);
complex.set_label(v0, shared_label);
complex.set_label(v1, shared_label);
```

---

## Troubleshooting

### Issue: "Simplex not found in equivalence manager"

**Cause**: Trying to glue a simplex that hasn't been added to the manager.

**Solution**: Ensure simplex exists before gluing:
```cpp
auto v0 = complex.add_simplex({});
auto v1 = complex.add_simplex({});
complex.glue_simplices(v0, v1);  // OK
```

### Issue: Glued simplices don't appear together in queries

**Cause**: Querying a specific simplex instead of the equivalence class.

**Solution**: Use the representative or query all members:
```cpp
auto rep = complex.find_representative(simplex_id);
auto members = complex.get_class_members(rep);
```

### Issue: Performance degrades with many gluing operations

**Cause**: Large equivalence classes reduce path compression effectiveness.

**Solution**: Consider the topology structure and minimize unnecessary gluing.

---

## Summary

Equivalence classes enable powerful non-Hausdorff topological constructions:

- **Glue simplices** to create quotient spaces
- **Maintain independent labels** and attributes
- **Efficient Union-Find** implementation
- **Use cases**: topological identification, symmetric meshes, mesh merging

For more advanced usage, see the API reference and examples.
