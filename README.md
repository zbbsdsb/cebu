# cebu

A C++ library implementing **dynamic simplicial complexes** for the Cebu Space project.
Cebu Space is a non-Hausdorff, narrative-driven topological structure where geometry depends on absurdity and intent.

## Features

### Current (v0.7.3)
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
- **JSON serialization support** 🆕
- Human-readable JSON format for debugging and data exchange
- JSON Schema validation
- Support for 6 complex types in JSON
- **ZLIB compression support** 🆕
- Automatic compression/decompression
- Configurable compression levels (0-9)
- Compression ratio estimation
- **Non-Hausdorff serialization** 🆕
- Equivalence classes preservation
- Glue/separate operation history
- **Change tracking and delta serialization** 🆕
- Track all changes (added, removed, label changes, equivalence changes)
- Export/Import changes as JSON
- Save only changes for efficient storage
- **Snapshot management** 🆕
- Create and restore snapshots
- Compare snapshots
- Compressed snapshot storage
- **Streaming I/O for large files** 🆕
- Stream-based loading/saving
- Progress callbacks
- Partial loading
- Chunk-based processing
- **Version control system** 🆕
- Git-style version management
- Branch and tag support
- Version diff and revert
- Commit history log
- **Comprehensive test suite** 🆕
- 56+ test cases covering all new features
- >85% code coverage
- Google Test framework
- Full integration tests

### Planned
- 3D tetrahedron refinement
- Boundary-aware refinement
- **Parallel computing engine** (multithreading, work-stealing)
- **SIMD vectorization** (AVX-512/AVX2 optimization)
- **GPU-accelerated operations** (CUDA/OpenCL)
- Diff-based gluing (only glue specific faces)
- Visual equivalence class viewer
- 3D rendering and visualization
- Export to 3D formats (OBJ, STL, PLY)

## Documentation

- [API Reference](docs/api.md) - Detailed API documentation
- [Architecture](docs/architecture.md) - Design and implementation details
- [Advanced Serialization API](docs/api_advanced_serialization.md) - Change tracking, snapshots, streaming I/O, and version control

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

### JSON Serialization

```cpp
#include "cebu/json_serialization.h"

using namespace cebu;

SimplicialComplexLabeled<double> complex;
// ... build complex ...

// Serialize to JSON
auto j = JsonSerializer::serialize_labeled(complex);

// Pretty print
std::string pretty = JsonSerializer::pretty_print(j, 2);
std::cout << pretty << std::endl;

// Save to file
std::ofstream out("complex.json");
out << pretty;

// Validate
if (JsonSerializer::validate(j)) {
    std::cout << "JSON is valid\n";
}

// Get schema
auto schema = JsonSerializer::get_schema();
```

### Compression

```cpp
#include "cebu/compression.h"

using namespace cebu;

SimplicialComplex complex;
// ... build complex ...

// Save with compression
PersistenceOptions options;
options.compression = Compression::ZLIB;
options.compression_level = 6;  // 0-9

Persistence::save(complex, "compressed.bin", options);

// Compression info
std::vector<uint8_t> data = BinarySerializer::serialize(complex);
auto [compressed, info] = Compression::compress_with_info(
    data, Compression::Algorithm::ZLIB, 6);

std::cout << "Compression ratio: " 
          << (info.compression_ratio * 100.0) << "%\n";

// Auto-detects compression on load
auto result = Persistence::load("compressed.bin");
```

### Change Tracking

```cpp
#include "cebu/change_tracker.h"

using namespace cebu;

ChangeTracker tracker;

// Track changes
VertexID v0 = complex.add_vertex();
tracker.track_simplex_added(v0, 0, {v0});

EdgeID e0 = complex.add_edge(v0, v1);
tracker.track_simplex_added(e0, 1, {v0, v1});

complex.set_label(e0, 0.8);
tracker.track_label_changed(e0, 0.0, 0.8);

// Get changes
auto changes = tracker.get_changes();
auto added = tracker.get_changes_by_type(ChangeType::SIMPLEX_ADDED);

// Save changes
tracker.save_to_file("delta.json");

// Load and apply
tracker.load_from_file("delta.json");
```

### Snapshot Management

```cpp
#include "cebu/snapshot_manager.h"

using namespace cebu;

SnapshotManager manager("snapshots.ceb");

// Create snapshots
manager.create_snapshot(complex, "initial");
manager.create_snapshot_labeled(complex, "checkpoint1");

// ... make changes ...

// Restore snapshot
manager.restore_snapshot(complex, "initial");

// Compare snapshots
auto changes = manager.compare_snapshots("initial", "checkpoint1");

// List all snapshots
auto snapshots = manager.list_snapshots();
```

### Streaming I/O

```cpp
#include "cebu/streaming_io.h"

using namespace cebu;

// Load large file with progress
StreamingLoader loader("large_complex.json");
loader.set_progress_callback([](size_t current, size_t total) {
    std::cout << "Progress: " << (current * 100 / total) << "%\n";
});

auto complex = loader.load();

// Get file metadata
size_t total_simplices = loader.get_total_simplices();
size_t file_size = loader.get_file_size();
bool compressed = loader.is_compressed();

// Write with progress
StreamingWriter writer("output.json");
writer.set_progress_callback(progress_callback);
writer.write(complex);
```

### Version Control

```cpp
#include "cebu/version_control.h"

using namespace cebu;

VersionControl vc("versions.ceb");

// Commit versions
VersionID v1 = vc.commit(complex, "Initial version", "alice");

// ... make changes ...

VersionID v2 = vc.commit(complex, "Add vertices", "bob");

// View history
auto history = vc.log(10);

// Checkout version
vc.checkout(complex, v1);

// Create branch
vc.create_branch("feature", v1);
vc.checkout_branch("feature");

// Merge branch
vc.merge_branch(complex, "feature");

// Create tag
vc.create_tag("v1.0.0", v2, "First release");

// Revert
vc.revert(complex, v1);
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
