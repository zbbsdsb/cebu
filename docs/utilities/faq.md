# Frequently Asked Questions (FAQ)

This FAQ addresses common questions about Cebu, troubleshooting issues, and provides solutions to known problems.

## Table of Contents

1. [Getting Started](#getting-started)
2. [Building and Installation](#building-and-installation)
3. [Core Concepts](#core-concepts)
4. [Performance Issues](#performance-issues)
5. [Serialization](#serialization)
6. [Spatial Indexing](#spatial-indexing)
7. [Labels and Narrative](#labels-and-narrative)
8. [Topology and Geometry](#topology-and-geometry)
9. [Common Errors](#common-errors)
10. [Advanced Topics](#advanced-topics)

---

## Getting Started

### Q: What is Cebu?

**A**: Cebu is a modern C++17 library for manipulating simplicial complexes. It provides:
- Efficient simplicial complex management
- Spatial indexing (BVH, Octree)
- Narrative-driven topology
- Advanced serialization and version control
- Non-Hausdorff topology (gluing)

### Q: What are the system requirements?

**A**:
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.14 or higher
- nlohmann/json (automatically fetched by CMake)
- ZLIB (optional, for compression)

### Q: Is Cebu thread-safe?

**A**: No, Cebu is not thread-safe by default. If you need multi-threading:
1. Use mutexes to protect shared access
2. Create separate complexes per thread
3. Use thread-local storage for queries

Future versions may include thread-safe variants.

### Q: Can I use Cebu with other libraries?

**A**: Yes! Cebu is designed to be compatible with:
- **Mesh libraries**: Convert to/from vertex/triangle formats
- **Serialization formats**: Import/export JSON, binary
- **Visualization**: Extract simplices for rendering

See [Integration Guide](integration_guide.md) for examples.

---

## Building and Installation

### Q: CMake fails with "nlohmann_json not found"

**A**: Cebu's CMake configuration should automatically fetch nlohmann/json. If it fails:

```bash
# Manually install nlohmann/json
git clone https://github.com/nlohmann/json.git
cd json
mkdir build && cd build
cmake .. -DJSON_BuildTests=OFF
sudo make install

# Then build Cebu
cd cebu
mkdir build && cd build
cmake ..
cmake --build .
```

### Q: Compilation fails with "undefined reference to..."

**A**: Ensure you're linking against `cebu_core`:

```cmake
# CMakeLists.txt
target_link_libraries(your_app cebu_core)
```

### Q: Windows build fails with "MSVC version too old"

**A**: Cebu requires C++17. Update Visual Studio to 2017 or later.

### Q: "C++17 features not supported" error

**A**: Ensure your compiler supports C++17 and it's enabled:

```bash
# GCC/Clang
g++ -std=c++17 -o my_app my_app.cpp -lcebu_core

# CMake
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

---

## Core Concepts

### Q: What is a simplicial complex?

**A**: A simplicial complex is a set of simplices (points, edges, triangles, tetrahedra, etc.) where:
- Any face of a simplex is also in the complex
- The intersection of two simplices is a face of both

**Example**:
```
Triangle = 3 vertices + 3 edges + 1 face
```

### Q: Why are vertices 0-simplices in Cebu?

**A**: This unified model simplifies the implementation:
- All simplices are treated uniformly
- Vertices are simplices with 0 faces
- Simplifies adjacency and containment queries

```cpp
// Add a vertex (0-simplex)
auto v0 = complex.add_simplex({});

// Add an edge (1-simplex)
auto e01 = complex.add_simplex({v0, v1});

// Add a triangle (2-simplex)
auto tri = complex.add_simplex({v0, v1, v2});
```

### Q: What's the difference between `add_simplex` and `add_vertex`?

**A**: In Cebu, they're the same! A vertex is just a 0-simplex:

```cpp
auto v0 = complex.add_simplex({});  // 0-simplex = vertex
auto v1 = complex.add_vertex();      // Same as add_simplex({})
```

### Q: How do I delete a simplex?

**A**: Use `remove_simplex` or `remove_simplex_cascade`:

```cpp
// Remove simplex and all its cofaces
complex.remove_simplex(simplex_id);

// Remove only this simplex (may break complex)
complex.remove_simplex(simplex_id, false);
```

**Warning**: Removing a simplex also removes all simplices that depend on it (cofaces).

---

## Performance Issues

### Q: My queries are slow. What can I do?

**A**: Use spatial indexing for 100-2000x speedup:

```cpp
// Bad: Linear search
for (auto id : all_simplices) {
    if (is_in_range(id, bbox)) { ... }
}

// Good: BVH spatial index
cebu::BVHTree tree;
tree.build(all_simplices, vertex_geometry, get_vertices);
auto results = tree.range_query(bbox);
```

See [Performance Guide](performance.md) for more tips.

### Q: Building the spatial index takes too long

**A**: Try a faster build strategy:

```cpp
// SAH: Best quality, slower build
cebu::BVHTree tree_sa(cebu::BVHBuildStrategy::SAH);

// Midpoint: Fastest build
cebu::BVHTree tree_fast(cebu::BVHBuildStrategy::MIDPOINT_SPLIT);

// For dynamic data with frequent updates:
cebu::BVHTree tree_dynamic(cebu::BVHBuildStrategy::MIDPOINT_SPLIT, 16, 12);
```

### Q: Memory usage is too high

**A**: Several strategies:

1. **Use binary serialization** instead of JSON:
   ```cpp
   cebu::serialization::save_to_binary_file(complex, "data.bin");
   ```

2. **Enable compression**:
   ```cpp
   cebu::Persistence::save_to_file(complex, "data.cjson", true);
   ```

3. **Use streaming I/O** for large files:
   ```cpp
   cebu::StreamingIO::load_chunked(complex, "large.cjson", 1024*1024);
   ```

4. **Reduce spatial index overhead**:
   ```cpp
   // Larger leaf nodes, shallower tree
   cebu::BVHTree tree(cebu::BVHBuildStrategy::MIDPOINT_SPLIT, 32, 10);
   ```

### Q: Serialization is very slow

**A**: Use binary format instead of JSON:

```cpp
// JSON (slow, human-readable)
cebu::serialization::save_to_json_file(complex, "data.json");

// Binary (fast, compact)
cebu::serialization::save_to_binary_file(complex, "data.bin");
```

**Performance gain**: 2-3x faster, 50% smaller files.

---

## Serialization

### Q: What file formats does Cebu support?

**A**:
- **JSON**: Human-readable, `.json` or `.cjson`
- **Binary**: Compact, efficient, `.bin`
- **Compressed**: ZLIB-compressed JSON/binary, `.cjson.gz`

### Q: How do I save/load a complex?

**A**:

```cpp
// Save to JSON
cebu::serialization::save_to_json_file(complex, "my_complex.json");

// Load from JSON
cebu::SimplicialComplex loaded;
cebu::serialization::load_from_json_file(loaded, "my_complex.json");

// Save to binary
cebu::serialization::save_to_binary_file(complex, "my_complex.bin");

// Load from binary
cebu::serialization::load_from_binary_file(loaded, "my_complex.bin");
```

### Q: Can I compress the files?

**A**: Yes, use the high-level Persistence API:

```cpp
// Save with compression
cebu::Persistence::save_to_file(complex, "data.cjson", true);

// Load (automatically detects compression)
cebu::Persistence::load_from_file(complex, "data.cjson");
```

### Q: How do I handle very large files (> 1GB)?

**A**: Use streaming I/O to load in chunks:

```cpp
cebu::SimplicialComplex complex;

// Load in 1MB chunks
cebu::StreamingIO::load_chunked(
    complex,
    "huge_file.cjson",
    1024 * 1024  // 1MB chunks
);

// Process as chunks are loaded
// Memory usage stays constant!
```

### Q: Can I save partial complexes?

**A**: Yes, extract and save specific components:

```cpp
// Extract component
auto components = cebu::TopologyOperations::get_connected_components(complex);
cebu::SimplicialComplex component;

// Copy only first component
for (auto id : components[0]) {
    const auto& simplex = complex.get_simplex(id);
    component.add_simplex(simplex.vertices());
}

// Save component
cebu::serialization::save_to_json_file(component, "component.json");
```

---

## Spatial Indexing

### Q: When should I use BVH vs Octree?

**A**:

| Use Case | Recommended | Reason |
|----------|------------|--------|
| General spatial queries | BVH (SAH) | Optimal quality |
| Ray intersection | BVH | Native support |
| Point containment | Octree | Efficient partitioning |
| Uniformly distributed data | Octree | Balanced tree |
| Frequent updates | BVH (Midpoint) | Fast rebuild |
| Very large datasets | BVH (HLBVH) | Optimized |

### Q: How do I add vertex coordinates?

**A**: Use VertexGeometry:

```cpp
cebu::VertexGeometry vg;

// Set vertex positions
vg.set_vertex(0, cebu::Point3D(0, 0, 0));
vg.set_vertex(1, cebu::Point3D(1, 0, 0));
vg.set_vertex(2, cebu::Point3D(0, 1, 0));

// Build spatial index
cebu::BVHTree tree;
tree.build(all_simplices, vg, get_simplex_vertices);
```

### Q: Do I need to rebuild the index after updates?

**A**: Depends on number of updates:

```cpp
// Few updates (1-10): Incremental update
tree.add_simplex(new_id);

// Many updates (10+): Rebuild
for (auto id : many_new_ids) {
    complex.add_simplex(/* ... */);
}
tree.rebuild();
```

### Q: What's the best build strategy?

**A**: Depends on your use case:

- **SAH**: Best query quality, slower build
- **Median Split**: Good balance
- **Midpoint Split**: Fastest build, good for dynamic data
- **HLBVH**: Best for very large datasets (>1M simplices)

### Q: Can I use spatial indexes without vertex geometry?

**A**: No, spatial indexes require 3D coordinates. However, you can use:
- **Distance functions**: If you can compute distances
- **Label ranges**: If you only need label-based queries
- **Adjacency queries**: For topological (not geometric) queries

---

## Labels and Narrative

### Q: How do I attach data to simplices?

**A**: Use the labeled simplicial complex:

```cpp
#include "cebu/simplicial_complex_labeled.h"

using namespace cebu;

// Define label type
struct MaterialLabel {
    float density;
    std::string name;
};

// Create labeled complex
SimplicialComplexLabeled<MaterialLabel> complex;

// Add with label
auto v0 = complex.add_simplex({}, {1000.0f, "Steel"});
auto tri = complex.add_simplex({v0, v1, v2}, {7850.0f, "Iron"});

// Query by label
auto iron_simplices = complex.get_simplices_with_label(
    [](const MaterialLabel& label) {
        return label.name == "Iron";
    }
);
```

### Q: What label types are supported?

**A**: Any copyable type! Common choices:
- `float`, `double`: Numeric values
- `std::string`: Text labels
- `Absurdity`: Interval-valued fuzzy numbers
- `struct`, `class`: Custom data

### Q: How do I update a label?

**A**: Use `set_label`:

```cpp
complex.set_label(simplex_id, new_label);
```

### Q: What is the timeline and story system?

**A**: Narrative features in Cebu include:
- **Timeline**: Manages time bounds and milestones
- **Story Events**: Time-based events that modify labels
- **Narrative Evolution**: Context-driven label changes

```cpp
cebu::Timeline timeline;
timeline.add_milestone(0.0f, "Start");
timeline.add_milestone(100.0f, "End");

cebu::StoryEvent event;
event.set_time(50.0f);
event.set_action(/* modify labels */);

timeline.add_event(event);
```

See [API Documentation](api.md) for details.

---

## Topology and Geometry

### Q: What's the Euler characteristic and how do I compute it?

**A**: The Euler characteristic is a topological invariant:

```cpp
#include "cebu/topology_operations.h"

int euler = cebu::TopologyOperations::compute_euler_characteristic(complex);
```

**Common values**:
- Sphere: 2
- Torus: 0
- Disk: 1 (has boundary)

### Q: How do I check if a mesh is a manifold?

**A**:

```cpp
bool is_manifold = cebu::TopologyOperations::is_manifold(complex);
if (!is_manifold) {
    std::cout << "Mesh has non-manifold regions" << std::endl;
}
```

### Q: How do I glue simplices together?

**A**:

```cpp
#include "cebu/simplicial_complex_non_hausdorff.h"

cebu::SimplicialComplexNonHausdorff complex;

// ... add simplices ...

// Glue two vertices
complex.glue_simplices(vertex_a, vertex_b);

// Check if glued
bool glued = complex.are_glued(vertex_a, vertex_b);
```

See [Equivalence Classes Guide](equivalence_classes.md) for details.

### Q: How do I compute the boundary?

**A**:

```cpp
auto boundary = cebu::TopologyOperations::compute_boundary(complex);

for (auto simplex_id : boundary) {
    std::cout << "Boundary simplex: " << simplex_id << std::endl;
}
```

### Q: Can I merge two complexes?

**A**: Yes, copy simplices from one to another:

```cpp
cebu::SimplicialComplex merged;

// Copy from complex1
complex1.for_each_simplex([&](cebu::SimplexID id) {
    const auto& simplex = complex1.get_simplex(id);
    merged.add_simplex(simplex.vertices());
});

// Copy from complex2
complex2.for_each_simplex([&](cebu::SimplexID id) {
    const auto& simplex = complex2.get_simplex(id);
    merged.add_simplex(simplex.vertices());
});
```

---

## Common Errors

### Q: "Simplex not found" error

**A**: This usually means:
1. The simplex doesn't exist (not added)
2. The simplex was deleted
3. Wrong simplex ID

**Solution**: Check if simplex exists first:

```cpp
if (complex.has_simplex(simplex_id)) {
    // Safe to use
} else {
    std::cout << "Simplex " << simplex_id << " doesn't exist" << std::endl;
}
```

### Q: "Duplicate simplex detected" error

**A**: Cebu prevents duplicate simplices. If you try to add the same simplex twice, you'll get the same ID back:

```cpp
auto e1 = complex.add_simplex({v0, v1});  // Returns ID 5
auto e2 = complex.add_simplex({v0, v1});  // Returns ID 5 (same as e1)
```

This is expected behavior - Cebu ensures uniqueness.

### Q: "Cannot remove simplex: has cofaces" error

**A**: You're trying to remove a simplex that's used by higher-dimensional simplices.

**Solution**: Use cascade delete or remove cofaces first:

```cpp
// Cascade delete (removes this simplex and all dependents)
complex.remove_simplex(simplex_id, true);

// Or remove cofaces first
auto cofaces = complex.get_cofaces(simplex_id);
for (auto coface_id : cofaces) {
    complex.remove_simplex(coface_id);
}
```

### Q: "Out of memory" error

**A**: Your complex is too large for available RAM.

**Solutions**:
1. Use streaming I/O to process in chunks
2. Enable compression
3. Use binary format (smaller memory footprint)
4. Split into smaller complexes

```cpp
// Process in chunks
cebu::StreamingIO::load_chunked(complex, "huge.cjson", 1024*1024);
```

### Q: "Label type mismatch" error

**A**: You're using the wrong label type for your complex.

**Solution**: Ensure label type matches:

```cpp
// Correct
SimplicialComplexLabeled<float> complex;
complex.add_simplex({}, 1.0f);  // float

// Wrong
SimplicialComplexLabeled<float> complex;
complex.add_simplex({}, "label");  // string != float
```

---

## Advanced Topics

### Q: Can I use Cebu with GPU acceleration?

**A**: Not directly, but you can:
1. Extract simplices to GPU-friendly format
2. Process on GPU
3. Import results back to Cebu

```cpp
// Export to GPU
std::vector<float> vertices;
std::vector<uint32_t> triangles;

complex.export_to_vertex_array(vertices, triangles);

// ... GPU processing ...

// Import back
complex.import_from_vertex_array(vertices, triangles);
```

### Q: How do I implement custom serialization?

**A**: Extend the serialization interface:

```cpp
class MyCustomSerializer {
public:
    void save(const SimplicialComplex& complex, std::ostream& out) {
        // Your custom format
    }

    void load(SimplicialComplex& complex, std::istream& in) {
        // Your custom format
    }
};
```

### Q: Can I use Cebu in real-time applications?

**A**: Yes, with proper optimization:
- Use spatial indexes
- Avoid JSON in hot paths
- Pre-build spatial indexes
- Use binary serialization
- Limit complex size

**Target performance**:
- Add simplex: < 0.001ms
- Spatial query: < 0.01ms
- Update: < 0.1ms

### Q: How do I version control my complexes?

**A**: Use the built-in version control:

```cpp
#include "cebu/version_control.h"

cebu::VersionControl vc(vc);

// Create branch
vc.create_branch("experiment");

// Commit changes
vc.commit("Added new triangles");

// Checkout branch
vc.checkout("experiment");

// Merge branches
vc.merge("experiment", "main");
```

See [Advanced Serialization Guide](api_advanced_serialization.md) for details.

### Q: Can I use Cebu with Python?

**A**: Cebu is C++ only, but you can:
1. Create Python bindings (pybind11, Boost.Python)
2. Use subprocess to call Cebu programs
3. Export to intermediate format (JSON, OBJ, PLY)

Example using pybind11:

```cpp
#include <pybind11/pybind11.h>

PYBIND11_MODULE(cebu_python, m) {
    py::class_<cebu::SimplicialComplex>(m, "SimplicialComplex")
        .def(py::init<>())
        .def("add_simplex", &cebu::SimplicialComplex::add_simplex);
        // ...
}
```

---

## Still Have Questions?

If you didn't find an answer here:

1. **Check the documentation**:
   - [Getting Started Guide](getting_started.md)
   - [API Documentation](api.md)
   - [Performance Guide](performance.md)

2. **Search existing issues**:
   - [GitHub Issues](https://github.com/yourusername/cebu/issues)

3. **Ask a new question**:
   - Open a GitHub issue
   - Include code snippets
   - Describe expected vs actual behavior

4. **Examples**:
   - Check `examples/` directory for working code

---

**Last Updated**: 2025-03-06
