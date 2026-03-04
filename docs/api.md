# Cebu Library API Documentation

## Overview

The **cebu** library is a C++ implementation of dynamic simplicial complexes for the Cebu Space project. Cebu Space is a non-Hausdorff, narrative-driven topological structure where geometry depends on absurdity and intent.

## Core Concepts

### Simplex

A simplex is the basic building block of combinatorial topology:

- **0-simplex**: A vertex (1 point)
- **1-simplex**: An edge (2 points)
- **2-simplex**: A triangle (3 points)
- **k-simplex**: A convex hull of k+1 points

### Simplicial Complex

A simplicial complex is a set of simplices that satisfies:

1. If a simplex is in the complex, all its faces are also in the complex
2. The intersection of two simplices is a common face (or empty)

## API Reference

### Types

```cpp
using SimplexID = uint64_t;      // Unique identifier for a simplex
using VertexID = uint32_t;       // Unique identifier for a vertex (alias for 0-simplex ID)
```

**Important**: Vertices are 0-simplices. `VertexID` is conceptually the same as the `SimplexID` of a 0-simplex. All simplices (including vertices) share a unified ID space.

### Simplex Class

The `Simplex` class represents a single simplex in the complex.

#### Constructors

```cpp
explicit Simplex(const std::vector<VertexID>& vertices, SimplexID id);
```

Creates a simplex with the given vertices and ID.

#### Methods

```cpp
SimplexID id() const;
```
Returns the unique ID of the simplex.

```cpp
size_t dimension() const;
```
Returns the dimension of the simplex (0 for vertex, 1 for edge, etc.).

```cpp
const std::vector<VertexID>& vertices() const;
```
Returns the list of vertices that make up this simplex.

```cpp
const std::unordered_set<SimplexID>& neighbors() const;
```
Returns the set of adjacent simplex IDs.

```cpp
void add_neighbor(SimplexID neighbor_id);
```
Adds a neighboring simplex.

```cpp
void remove_neighbor(SimplexID neighbor_id);
```
Removes a neighboring simplex.

```cpp
bool has_neighbor(SimplexID neighbor_id) const;
```
Checks if the simplex is adjacent to the given simplex.

### SimplicialComplex Class

The `SimplicialComplex` class manages a collection of simplices.

#### Constructors

```cpp
SimplicialComplex();
```
Creates an empty simplicial complex.

#### Adding Elements

```cpp
VertexID add_vertex();
```
Adds a new vertex (0-simplex) and returns its ID. **Note**: This creates a 0-simplex with vertices={id}, which means the vertex ID equals its simplex ID.

```cpp
SimplexID add_edge(VertexID v1, VertexID v2);
```
Adds an edge between two vertices and returns its ID.

```cpp
SimplexID add_triangle(VertexID v1, VertexID v2, VertexID v3);
```
Adds a triangle with three vertices and returns its ID.

```cpp
SimplexID add_simplex(const std::vector<VertexID>& vertices);
```
Adds a k-simplex with the given vertices and returns its ID.

#### Removing Elements

```cpp
bool remove_simplex(SimplexID simplex_id, bool cascade = false);
```
Removes a simplex from the complex. If `cascade` is true, also removes all simplices that contain this simplex as a face. Returns `true` if the simplex was removed, `false` if it didn't exist.

```cpp
bool remove_vertex(VertexID vertex_id, bool cascade = true);
```
Removes a vertex from the complex. If `cascade` is true (default), also removes all simplices that contain this vertex. Returns `true` if the vertex was removed, `false` if it didn't exist.

#### Querying Elements

```cpp
const std::vector<VertexID>& get_vertices() const;
```
Returns all vertex IDs.

```cpp
const std::unordered_map<SimplexID, Simplex>& get_simplices() const;
```
Returns all simplices in the complex.

```cpp
std::vector<SimplexID> get_simplices_of_dimension(size_t dimension) const;
```
Returns all simplices of a specific dimension.

```cpp
bool has_simplex(SimplexID id) const;
```
Checks if a simplex with the given ID exists.

```cpp
const Simplex& get_simplex(SimplexID id) const;
```
Returns the simplex with the given ID. Throws `std::out_of_range` if the simplex doesn't exist.

```cpp
std::vector<SimplexID> get_simplices_containing_vertex(VertexID vertex_id) const;
```
Returns all simplices that contain the specified vertex.

```cpp
std::vector<SimplexID> get_adjacent_simplices(SimplexID simplex_id) const;
```
Returns all simplices adjacent to the specified simplex.

```cpp
std::vector<SimplexID> get_facets(SimplexID simplex_id) const;
```
Returns all faces (boundary simplices of dimension-1) of the specified simplex.

```cpp
size_t simplex_count() const;
```
Returns the total number of simplices.

```cpp
size_t vertex_count() const;
```
Returns the total number of vertices.

## Usage Examples

### Creating a Simple Triangle

```cpp
#include "cebu/simplicial_complex.h"

using namespace cebu;

SimplicialComplex complex;

// Add vertices
VertexID v0 = complex.add_vertex();
VertexID v1 = complex.add_vertex();
VertexID v2 = complex.add_vertex();

// Add triangle
SimplexID tri = complex.add_triangle(v0, v1, v2);

// Query properties
std::cout << "Simplex count: " << complex.simplex_count() << std::endl;
```

### Dynamic Operations

```cpp
SimplicialComplex complex;

VertexID v0 = complex.add_vertex();
VertexID v1 = complex.add_vertex();
VertexID v2 = complex.add_vertex();

SimplexID e01 = complex.add_edge(v0, v1);
SimplexID e12 = complex.add_edge(v1, v2);
SimplexID tri = complex.add_triangle(v0, v1, v2);

// Remove the middle vertex with cascade
complex.remove_vertex(v1, true);
// This will remove e01, e12, and tri

// Add new simplices
VertexID v3 = complex.add_vertex();
SimplexID e02 = complex.add_edge(v0, v2);
```

### Querying Adjacency

```cpp
SimplicialComplex complex;

VertexID v0 = complex.add_vertex();
VertexID v1 = complex.add_vertex();
VertexID v2 = complex.add_vertex();

SimplexID tri = complex.add_triangle(v0, v1, v2);
SimplexID e01 = complex.add_edge(v0, v1);

// Get simplices adjacent to the edge
auto adjacent = complex.get_adjacent_simplices(e01);
// adjacent will contain the triangle ID
```

## Building

The library uses CMake for building:

```bash
mkdir build && cd build
cmake ..
make
```

On Windows with Visual Studio:

```bash
mkdir build
cmake -S . -B build
cmake --build build --config Release
```

## Testing

Run the test suite:

```bash
./build/Release/test_basic
./build/Release/test_dynamic
```

Or using CMake:

```bash
cd build
ctest
```

## Implementation Details

### Unified Simplex Model

**Innovation**: Vertices are themselves 0-simplices (dimension=0, vertices={id}). This design:

- Eliminates ID conflicts between vertices and higher-dim simplices
- Provides a unified framework for all simplex operations
- Ensures mathematical consistency with combinatorial topology
- Simplifies label system (no need for separate vertex/simplex label stores)

### Data Structures

- **Simplices**: All simplices (including 0-simplices/vertices) stored in `std::unordered_map<SimplexID, Simplex>`
- **Vertex mappings**: `std::unordered_map<VertexID, std::unordered_set<SimplexID>>` for reverse lookup
- **Unified ID space**: `SimplexID` counter shared by all simplex types

### Adjacency Relations

Simplices maintain adjacency relations through `std::unordered_set<SimplexID>`. Two simplices are considered adjacent if they share at least one vertex.

### Duplicate Prevention

Adding a simplex with the same vertices as an existing simplex returns the ID of the existing simplex instead of creating a duplicate.

### Cascade Deletion

When removing a simplex with `cascade=true`, the library recursively removes all simplices that contain the removed simplex as a face.

## Label System

The label system allows attaching arbitrary data to simplices for semantic enrichment.

### Label Types

#### Default Label System (Numeric Labels)

```cpp
using SimplicialComplexDouble = SimplicialComplexLabeled<double>;
```

For numeric labels (int, float, double, etc.).

#### Absurdity Label System

```cpp
using SimplicialComplexAbsurdity = SimplicialComplexLabeled<Absurdity, AbsurdityLabelSystem>;
```

For interval-valued fuzzy numbers representing absurdity metrics.

### SimplicialComplexLabeled Class

Template class that extends `SimplicialComplex` with label support.

#### Template Parameters

```cpp
template<typename LabelType, typename LabelSystemType = DefaultLabelSystem<LabelType>>
class SimplicialComplexLabeled;
```

- `LabelType`: Type of the label (e.g., `double`, `Absurdity`)
- `LabelSystemType`: Label system implementation (defaults to `DefaultLabelSystem<LabelType>`)

#### Label Operations

```cpp
void set_label(SimplexID simplex_id, const LabelType& label);
```
Sets a label for a simplex.

```cpp
std::optional<LabelType> get_label(SimplexID simplex_id) const;
```
Gets the label for a simplex. Returns empty optional if not set.

```cpp
bool remove_label(SimplexID simplex_id);
```
Removes the label from a simplex.

```cpp
bool has_label(SimplexID simplex_id) const;
```
Checks if a simplex has a label.

```cpp
size_t labeled_count() const;
```
Returns the number of labeled simplices.

```cpp
void clear_labels();
```
Removes all labels.

#### Label Queries

```cpp
std::vector<SimplexID> find_by_label(Predicate predicate) const;
```
Finds all simplices with labels matching a predicate.

```cpp
std::vector<SimplexID> find_high_labels(double threshold = 0.7) const;
```
Finds simplices with numeric labels above threshold (only for arithmetic types).

```cpp
std::vector<SimplexID> find_low_labels(double threshold = 0.3) const;
```
Finds simplices with numeric labels below threshold (only for arithmetic types).

```cpp
std::vector<SimplexID> find_labels_in_range(double min, double max) const;
```
Finds simplices with numeric labels in range (only for arithmetic types).

#### Absurdity-Specific Queries

```cpp
std::vector<SimplexID> find_high_absurdity(double threshold = 0.7) const;
```
Finds simplices with absurdity midpoint above threshold.

```cpp
std::vector<SimplexID> find_low_absurdity(double threshold = 0.3) const;
```
Finds simplices with absurdity midpoint below threshold.

```cpp
std::vector<SimplexID> find_high_uncertainty(double threshold = 0.3) const;
```
Finds simplices with uncertainty (range width) above threshold.

### Absurdity Class

Represents interval-valued fuzzy numbers for narrative-driven topology.

```cpp
class Absurdity {
public:
    Absurdity(double lower = 0.5, double upper = 0.5, double confidence = 1.0);

    double lower() const;           // Lower bound of absurdity [0,1]
    double upper() const;           // Upper bound of absurdity [0,1]
    double confidence() const;       // Confidence in estimate [0,1]
    double midpoint() const;        // Midpoint of range
    double uncertainty() const;      // Width of range (upper - lower)

    void evolve(const AbsurdityContext& ctx);  // Update based on context
};
```

### AbsurdityContext

Environmental factors influencing absurdity evolution.

```cpp
struct AbsurdityContext {
    double surprisal;              // Unexpectedness [0,1]
    double logical_deviation;      // Deviation from logic [0,1]
    double user_laughter;          // User laughter intensity [0,1]
    double narrative_tension;     // Narrative tension [0,1]
    double dt;                     // Time step
};
```

### Usage Example: Numeric Labels

```cpp
#include "cebu/simplicial_complex_labeled.h"

using namespace cebu;

SimplicialComplexDouble complex;

VertexID v0 = complex.add_vertex();
VertexID v1 = complex.add_vertex();
SimplexID tri = complex.add_triangle(v0, v1, complex.add_vertex());

// Set labels
complex.set_label(v0, 0.1);
complex.set_label(v1, 0.9);
complex.set_label(tri, 0.5);

// Query labels
auto high = complex.find_high_labels(0.7);  // Returns {v1}
```

### Usage Example: Absurdity Labels

```cpp
#include "cebu/simplicial_complex_labeled.h"
#include "cebu/absurdity.h"

using namespace cebu;

SimplicialComplexAbsurdity complex;

VertexID v0 = complex.add_vertex();
VertexID v1 = complex.add_vertex();

// Set absurdity labels
Absurdity a1(0.6, 0.8, 0.9);  // High absurdity, high confidence
complex.set_label(v0, a1);

Absurdity a2(0.1, 0.3, 0.5);  // Low absurdity, low confidence
complex.set_label(v1, a2);

// Find high absurdity regions
auto high = complex.label_system().find_high_absurdity(0.5);

// Evolve absurdity
AbsurdityContext ctx(0.9, 0.8, 0.0, 0.5, 0.1);
complex.label_system().update_all(ctx);
```

## Future Extensions

- Morph operations for dynamic topology changes
- Non-Hausdorff topology support (gluing operations)
- Event system for external integration
- Serialization/deserialization
- Narrative layer for story-driven topology
- Visualization tools

## License

MIT License - See LICENSE file for details.
