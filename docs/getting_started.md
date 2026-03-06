# Getting Started with Cebu

This guide will help you get started with Cebu, from installation to building your first simplicial complex.

## Table of Contents

1. [Installation](#installation)
2. [Your First Complex](#your-first-complex)
3. [Working with Labels](#working-with-labels)
4. [Common Operations](#common-operations)
5. [Serialization](#serialization)
6. [Next Steps](#next-steps)

---

## Installation

### Prerequisites

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.14 or higher
- Git (optional, for cloning)

### Building from Source

#### Linux / macOS

```bash
# Clone the repository
git clone https://github.com/yourusername/cebu.git
cd cebu

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build the library
cmake --build . -j$(nproc)

# Run tests (optional)
ctest --output-on-failure
```

#### Windows (Visual Studio)

```batch
REM Clone the repository
git clone https://github.com/yourusername/cebu.git
cd cebu

REM Create build directory
mkdir build
cd build

REM Configure with CMake (adjust generator as needed)
cmake ..

REM Build the library
cmake --build . --config Release

REM Run tests (optional)
ctest -C Release --output-on-failure
```

### Building Your Project with Cebu

Create a `CMakeLists.txt` for your project:

```cmake
cmake_minimum_required(VERSION 3.14)
project(MyProject)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

# Find Cebu
find_package(cebu REQUIRED)

# Link to Cebu
add_executable(my_app main.cpp)
target_link_libraries(my_app cebu_core)
```

---

## Your First Complex

Let's build a simple triangle mesh.

### Step 1: Include Headers

```cpp
#include "cebu/simplicial_complex.h"
#include <iostream>
```

### Step 2: Create a Complex

```cpp
int main() {
    // Create an empty simplicial complex
    cebu::SimplicialComplex complex;

    std::cout << "Created simplicial complex" << std::endl;
    return 0;
}
```

### Step 3: Add Vertices

In Cebu, vertices are 0-simplices (simplices with 0 faces).

```cpp
int main() {
    cebu::SimplicialComplex complex;

    // Add three vertices
    auto v0 = complex.add_simplex({});
    auto v1 = complex.add_simplex({});
    auto v2 = complex.add_simplex({});

    std::cout << "Added vertices: " << v0 << ", " << v1 << ", " << v2 << std::endl;
    return 0;
}
```

**Note**: The empty brace `{}` indicates a 0-simplex (vertex).

### Step 4: Add an Edge

An edge is a 1-simplex connecting two vertices.

```cpp
// Add edge between v0 and v1
auto e01 = complex.add_simplex({v0, v1});

std::cout << "Added edge: " << e01 << std::endl;
```

### Step 5: Add a Triangle

A triangle is a 2-simplex with three vertices.

```cpp
// Add triangle with vertices v0, v1, v2
auto triangle = complex.add_simplex({v0, v1, v2});

std::cout << "Added triangle: " << triangle << std::endl;
```

### Complete Example

```cpp
#include "cebu/simplicial_complex.h"
#include <iostream>

int main() {
    cebu::SimplicialComplex complex;

    // Add vertices
    auto v0 = complex.add_simplex({});
    auto v1 = complex.add_simplex({});
    auto v2 = complex.add_simplex({});

    std::cout << "Added vertices: " << v0 << ", " << v1 << ", " << v2 << std::endl;

    // Add edges
    auto e01 = complex.add_simplex({v0, v1});
    auto e12 = complex.add_simplex({v1, v2});
    auto e20 = complex.add_simplex({v2, v0});

    std::cout << "Added edges: " << e01 << ", " << e12 << ", " << e20 << std::endl;

    // Add triangle
    auto triangle = complex.add_simplex({v0, v1, v2});

    std::cout << "Added triangle: " << triangle << std::endl;

    // Print statistics
    std::cout << "\nStatistics:" << std::endl;
    std::cout << "Vertices: " << complex.vertex_count() << std::endl;
    std::cout << "Edges: " << complex.simplex_count(1) << std::endl;
    std::cout << "Triangles: " << complex.simplex_count(2) << std::endl;

    return 0;
}
```

**Expected Output**:
```
Added vertices: 0, 1, 2
Added edges: 3, 4, 5
Added triangle: 6

Statistics:
Vertices: 3
Edges: 3
Triangles: 1
```

---

## Working with Labels

Labels allow you to attach data to simplices and query them efficiently.

### Creating a Labeled Complex

```cpp
#include "cebu/simplicial_complex_labeled.h"
#include <iostream>

int main() {
    using namespace cebu;

    // Create a labeled complex with float labels
    SimplicialComplexLabeled<float> complex;

    // Add vertices with temperature labels
    auto v0 = complex.add_simplex({}, 20.5f);  // 20.5°C
    auto v1 = complex.add_simplex({}, 25.3f);  // 25.3°C
    auto v2 = complex.add_simplex({}, 22.1f);  // 22.1°C

    // Add a triangle with average temperature
    auto triangle = complex.add_simplex({v0, v1, v2}, 22.6f);

    // Query by label range
    auto hot_vertices = complex.get_simplices_with_label(
        [](float temp) { return temp >= 23.0f; }
    );

    std::cout << "Hot vertices: " << hot_vertices.size() << std::endl;

    return 0;
}
```

### Numeric Label Queries

```cpp
// Get simplices with label > threshold
auto high = complex.get_high_labeled_simplices(10.0f);

// Get simplices with label < threshold
auto low = complex.get_low_labeled_simplices(10.0f);

// Get simplices in range
auto in_range = complex.get_simplices_in_label_range(5.0f, 15.0f);

// Get maximum labeled simplex
auto max_label = complex.get_max_label_simplex();
```

---

## Common Operations

### Querying Adjacent Simplices

```cpp
// Get all neighbors of a simplex
auto neighbors = complex.get_neighbors(simplex_id);

// Get all cofaces (simplices that contain this simplex)
auto cofaces = complex.get_cofaces(simplex_id);

// Get facets (immediate faces)
auto facets = complex.get_facets(simplex_id);
```

### Topological Queries

```cpp
#include "cebu/topology_operations.h"

// Check if complex is a manifold
bool is_manifold = topology_operations::is_manifold(complex);

// Compute Euler characteristic
int euler = topology_operations::compute_euler_characteristic(complex);

// Get connected components
auto components = topology_operations::get_connected_components(complex);
```

### Removing Simplices

```cpp
// Remove a simplex (also removes all its cofaces)
complex.remove_simplex(simplex_id);

// Cascade delete removes simplex and all dependent simplices
complex.remove_simplex_cascade(simplex_id);
```

### Iterating Over Simplices

```cpp
// Iterate over all simplices of dimension 2
for (const auto& simplex_id : complex.get_simplices_of_dimension(2)) {
    std::cout << "Triangle: " << simplex_id << std::endl;
}

// Iterate over all simplices
complex.for_each_simplex([](cebu::SimplexID id) {
    std::cout << "Simplex: " << id << std::endl;
});
```

---

## Serialization

Save and load your complexes to/from files.

### JSON Serialization

```cpp
#include "cebu/json_serialization.h"

// Save to JSON
cebu::serialization::save_to_json_file(complex, "my_complex.json");

// Load from JSON
cebu::SimplicialComplex loaded;
cebu::serialization::load_from_json_file(loaded, "my_complex.json");
```

### Binary Serialization

```cpp
#include "cebu/binary_serialization.h"

// Save to binary (more compact)
cebu::serialization::save_to_binary_file(complex, "my_complex.bin");

// Load from binary
cebu::SimplicialComplex loaded;
cebu::serialization::load_from_binary_file(loaded, "my_complex.bin");
```

### With Compression

```cpp
#include "cebu/persistence.h"

// Save with compression
cebu::Persistence::save_to_file(complex, "my_complex.cjson", true);

// Load compressed file
cebu::Persistence::load_from_file(complex, "my_complex.cjson");
```

---

## Common Pitfalls

### 1. Forgetting to Add Vertices

**Wrong**:
```cpp
auto edge = complex.add_simplex({0, 1});  // Error: vertices 0 and 1 don't exist!
```

**Correct**:
```cpp
auto v0 = complex.add_simplex({});
auto v1 = complex.add_simplex({});
auto edge = complex.add_simplex({v0, v1});
```

### 2. Adding Duplicate Simplices

Cebu automatically prevents duplicates:

```cpp
auto e1 = complex.add_simplex({v0, v1});  // Returns ID 5
auto e2 = complex.add_simplex({v0, v1});  // Returns ID 5 (same as e1)
```

### 3. Removing Used Simplices

Removing a simplex also removes all simplices that depend on it:

```cpp
auto v0 = complex.add_simplex({});
auto v1 = complex.add_simplex({});
auto edge = complex.add_simplex({v0, v1});
auto triangle = complex.add_simplex({v0, v1, v2});

complex.remove_simplex(v0);  // Also removes edge and triangle!
```

### 4. Label Type Mismatch

When creating labeled complexes, specify the label type:

```cpp
// Wrong: label type doesn't match
SimplicialComplexLabeled<float> complex;
complex.add_simplex({}, "label");  // Error: expected float, got string

// Correct
SimplicialComplexLabeled<float> complex;
complex.add_simplex({}, 42.0f);
```

---

## Next Steps

Now that you know the basics, explore more advanced features:

- [API Documentation](api.md) - Complete API reference
- [Spatial Indexing](spatial_indexing.md) - Accelerate spatial queries
- [Narrative Features](api.md#narrative-features) - Timeline-driven topology
- [Refinement System](api.md#refinement) - Adaptive mesh refinement
- [Event System](api.md#event-system) - Reactive programming
- [Architecture Overview](architecture.md) - Design and internals

---

## Full Example: Building a Tetrahedron

```cpp
#include "cebu/simplicial_complex.h"
#include "cebu/simplicial_complex_labeled.h"
#include "cebu/json_serialization.h"
#include <iostream>

int main() {
    // Create a labeled complex
    using namespace cebu;
    SimplicialComplexLabeled<float> complex;

    // Add 4 vertices (corners of a tetrahedron)
    auto v0 = complex.add_simplex({}, 0.0f);
    auto v1 = complex.add_simplex({}, 1.0f);
    auto v2 = complex.add_simplex({}, 2.0f);
    auto v3 = complex.add_simplex({}, 3.0f);

    // Add 6 edges
    complex.add_simplex({v0, v1}, 4.0f);
    complex.add_simplex({v0, v2}, 5.0f);
    complex.add_simplex({v0, v3}, 6.0f);
    complex.add_simplex({v1, v2}, 7.0f);
    complex.add_simplex({v1, v3}, 8.0f);
    complex.add_simplex({v2, v3}, 9.0f);

    // Add 4 faces (triangles)
    auto f012 = complex.add_simplex({v0, v1, v2}, 10.0f);
    auto f013 = complex.add_simplex({v0, v1, v3}, 11.0f);
    auto f023 = complex.add_simplex({v0, v2, v3}, 12.0f);
    auto f123 = complex.add_simplex({v1, v2, v3}, 13.0f);

    // Add the tetrahedron (3-simplex)
    auto tetra = complex.add_simplex({v0, v1, v2, v3}, 14.0f);

    // Print statistics
    std::cout << "Tetrahedron Statistics:" << std::endl;
    std::cout << "Vertices: " << complex.vertex_count() << std::endl;
    std::cout << "Edges: " << complex.simplex_count(1) << std::endl;
    std::cout << "Faces: " << complex.simplex_count(2) << std::endl;
    std::cout << "Tetrahedra: " << complex.simplex_count(3) << std::endl;

    // Query by label
    auto high_labeled = complex.get_simplices_with_label(
        [](float label) { return label >= 10.0f; }
    );
    std::cout << "\nSimplices with label >= 10: " << high_labeled.size() << std::endl;

    // Save to JSON
    serialization::save_to_json_file(complex, "tetrahedron.json");
    std::cout << "\nSaved to tetrahedron.json" << std::endl;

    return 0;
}
```

Compile and run:
```bash
g++ -std=c++17 -I include -L build -o tetrahedron tetrahedron.cpp -lcebu_core
./tetrahedron
```

---

Congratulations! You've completed the Getting Started guide. Continue exploring Cebu's advanced features in the documentation.
