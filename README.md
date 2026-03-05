# cebu

A C++ library implementing **dynamic simplicial complexes** for the Cebu Space project.
Cebu Space is a non-Hausdorff, narrative-driven topological structure where geometry depends on absurdity and intent.

## Features

### Current (v0.7.0)
- Dynamic addition/removal of simplices (vertices, edges, faces, etc.)
- Cascade deletion for maintaining complex consistency
- Query simplices by dimension, connectivity, and containment
- Adjacency relationship management
- Contextual labels (e.g., absurdity, narrative tension)
- Query simplices by label
- Morphism operations to reshape the complex based on rules
- **Non-Hausdorff topology support (gluing operations)** ✨
- Event system for external integration
- Serialization/deserialization
- Binary serialization with narrative support
- Undo/redo system with command history
- **Fractal dimension dynamic adjustment (Refinement/Coarsening)**
- Adaptive mesh refinement based on labels
- Multi-resolution analysis support
- **Event system for external integration** 🆕
- Publish-subscribe pattern for topology changes
- 12 event types (add, remove, label change, refinement, etc.)
- Automatic event triggering in all operations
- **Complete persistence and file I/O system** 🆕
- Save/load for all complex types (basic, labeled, narrative, refinement)
- Automatic file format detection (.bin, .ceb, .json)
- File metadata extraction
- File validation and integrity checking
- Comprehensive test suite

### Planned
- 3D tetrahedron refinement
- Boundary-aware refinement
- Spatial indexing (BVH, Octree) for performance
- Parallel refinement with multithreading
- GPU-accelerated refinement
- Diff-based gluing (only glue specific faces)
- Visual equivalence class viewer
- 3D rendering and visualization
- Export to 3D formats (OBJ, STL, PLY)

## Documentation

- [API Reference](docs/api.md) - Detailed API documentation
- [Architecture](docs/architecture.md) - Design and implementation details

## Quick Start

```cpp
#include "cebu/simplicial_complex.h"
#include "cebu/persistence.h"

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

// Save to file
Persistence::save(complex, "my_complex.bin");
```

### Adaptive Mesh Refinement

```cpp
#include "cebu/refinement.h"

using namespace cebu;

SimplicialComplexRefinement<double> complex;

// Create a triangle
VertexID v0 = complex.add_vertex();
VertexID v1 = complex.add_vertex();
VertexID v2 = complex.add_vertex();
SimplexID tri = complex.add_triangle(v0, v1, v2);

// Set label (e.g., curvature)
complex.set_label(tri, 0.9);

// Refine adaptively based on label
RefinementOptions<double> options;
options.label_strategy = LabelInheritanceStrategy::INHERIT_COPY;

auto result = complex.adaptive_refine(
    [](const double& label, SimplexID id) {
        return label > 0.5; // Refine high-value regions
    },
    options
);

std::cout << "Refined " << result.new_simplices_count << " simplices\n";
```

### Event System

```cpp
#include "cebu/simplicial_complex_labeled_events.h"

using namespace cebu;

SimplicialComplexLabeledEvents<double> complex;

// Register callbacks
complex.register_callback(EventType::ON_ADD_SIMPLEX,
    [](const EventData& event, const double* label) {
        std::cout << "Added simplex " << event.simplex_id << "\n";
    });

complex.register_callback(EventType::ON_LABEL_CHANGE,
    [](const EventData& event, const double* label) {
        std::cout << "Label changed for simplex " << event.simplex_id;
        if (label) {
            std::cout << " to " << *label;
        }
        std::cout << "\n";
    });

// Operations automatically trigger events
VertexID v0 = complex.add_vertex();  // Triggers ON_ADD_SIMPLEX
complex.set_label(v0, 0.8);         // Triggers ON_LABEL_CHANGE
```

### Persistence

```cpp
#include "cebu/persistence.h"

using namespace cebu;

SimplicialComplexLabeled<double> complex;

// ... build complex ...

// Save to file (auto-detects format from extension)
Persistence::save_labeled(complex, "data.bin");      // Binary
Persistence::save_labeled(complex, "data.json");    // JSON

// Load from file
auto result = Persistence::load_labeled<double>("data.bin");
if (result.success) {
    std::cout << "Loaded " << result.complex.vertex_count() << " vertices\n";
}

// Get metadata without loading
auto metadata = Persistence::get_metadata("data.bin");
std::cout << "File contains " << metadata.simplex_count << " simplices\n";

// Validate file
if (Persistence::validate_file("data.bin")) {
    std::cout << "File is valid\n";
}
```

## Build

This project uses CMake. To build:

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
# Build tests
cmake --build build --config Release

# Run all tests
./build/Release/test_basic
./build/Release/test_dynamic
```

Or use CMake:

```bash
cd build
ctest
```

## Project Structure

```
cebu/
├── include/cebu/       # Public headers
├── src/                # Implementation
├── tests/              # Unit tests
├── docs/               # Documentation
├── examples/           # Example code
├── CMakeLists.txt      # Build config
└── README.md           # This file
```

## License

MIT License - See [LICENSE](LICENSE) for details.

## Contributing

Contributions are welcome! Please see the documentation for details.
