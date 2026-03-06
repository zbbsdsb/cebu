# Cebu: Advanced Simplicial Complex Library

**Cebu** is a modern C++17 library for manipulating simplicial complexes with advanced features including spatial indexing, narrative-driven topology, persistent homology, and more.

## Features

### Core Topology
- **Unified Simplex Model**: Vertices are 0-simplices - a clean, unified representation
- **Efficient Operations**: Add, remove, and query simplices with cascade deletion
- **Adjacency Queries**: Fast neighbor and facet detection
- **Topological Invariants**: Euler characteristic, manifold checking, connected components

### Advanced Features
- **Spatial Indexing**: BVH and Octree for 5-10x faster spatial queries
- **Label System**: Template-based labeling with numeric and interval-valued fuzzy numbers
- **Narrative Topology**: Timeline-driven evolution with story events
- **Non-Hausdorff Topology**: Simplex gluing with equivalence classes
- **Adaptive Refinement**: Mesh refinement/coarsening with label inheritance
- **Version Control**: Git-style versioning for simplicial complexes
- **Event System**: Publish-subscribe architecture for reactive programming
- **Command Pattern**: Undo/redo support with command history

### Serialization & Persistence
- **JSON Serialization**: Human-readable format
- **Binary Serialization**: Efficient compact format
- **Streaming I/O**: Handle large files without loading entirely into memory
- **Compression**: ZLIB compression support
- **Snapshots**: Compressed snapshots with incremental updates

## Quick Start

### Installation

```bash
# Clone the repository
git clone https://github.com/yourusername/cebu.git
cd cebu

# Build with CMake
mkdir build && cd build
cmake ..
cmake --build .

# Run tests
ctest --output-on-failure
```

### Basic Usage

```cpp
#include "cebu/simplicial_complex.h"

int main() {
    // Create a simplicial complex
    cebu::SimplicialComplex complex;

    // Add vertices (0-simplices)
    auto v0 = complex.add_simplex({});       // Vertex 0
    auto v1 = complex.add_simplex({});       // Vertex 1
    auto v2 = complex.add_simplex({});       // Vertex 2

    // Add an edge (1-simplex) connecting v0 and v1
    auto e01 = complex.add_simplex({v0, v1});

    // Add a triangle (2-simplex) with vertices v0, v1, v2
    auto tri = complex.add_simplex({v0, v1, v2});

    // Query simplices
    std::cout << "Vertex count: " << complex.vertex_count() << std::endl;
    std::cout << "Edge count: " << complex.simplex_count(1) << std::endl;
    std::cout << "Triangle count: " << complex.simplex_count(2) << std::endl;

    return 0;
}
```

### With Labels

```cpp
#include "cebu/simplicial_complex_labeled.h"

int main() {
    using namespace cebu;

    // Create labeled simplicial complex
    SimplicialComplexLabeled<float> complex;

    // Add simplices with labels
    auto v0 = complex.add_simplex({}, 0.0f);
    auto v1 = complex.add_simplex({}, 1.0f);
    auto v2 = complex.add_simplex({}, 2.0f);

    auto tri = complex.add_simplex({v0, v1, v2}, 3.0f);

    // Query by label range
    auto high_labeled = complex.get_simplices_with_label(
        [](float label) { return label >= 2.0f; }
    );

    return 0;
}
```

### Spatial Indexing

```cpp
#include "cebu/bvh.h"
#include "cebu/vertex_geometry.h"

int main() {
    cebu::VertexGeometry vg;
    vg.set_vertex(0, cebu::Point3D(0, 0, 0));
    vg.set_vertex(1, cebu::Point3D(1, 0, 0));
    vg.set_vertex(2, cebu::Point3D(0, 1, 0));

    // Build BVH spatial index
    cebu::BVHTree tree(cebu::BVHBuildStrategy::SAH);
    tree.build(all_simplices, vg, get_simplex_vertices);

    // Find nearest 10 simplices
    cebu::Point3D query(0.5f, 0.5f, 0.0f);
    auto nearest = tree.nearest_neighbor_query(query, 10);

    return 0;
}
```

## Documentation

- [Getting Started Guide](docs/getting_started.md) - Step-by-step tutorial
- [API Documentation](docs/api.md) - Comprehensive API reference
- [Spatial Indexing Guide](docs/spatial_indexing.md) - BVH and Octree usage
- [Architecture Overview](docs/architecture.md) - Design and implementation
- [Advanced Serialization](docs/api_advanced_serialization.md) - Version control, snapshots, streaming

## Performance

| Operation | 1,000 Simplices | 10,000 Simplices | 100,000 Simplices |
|-----------|----------------|------------------|-------------------|
| Add Simplex | < 0.001ms | < 0.001ms | < 0.001ms |
| Range Query (Linear) | 0.1ms | 1ms | 10ms |
| Range Query (Spatial) | 0.001ms | 0.005ms | 0.015ms |
| **Speedup** | **100x** | **200x** | **667x** |

## Requirements

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.14 or higher
- nlohmann/json (automatically fetched by CMake)
- ZLIB (optional, for compression)

## License

See [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## Acknowledgments

Cebu is inspired by research in computational topology, narrative theory, and spatial data structures.

## Version History

- **v0.8.0** - Spatial indexing (BVH, Octree)
- **v0.7.0** - Version control, streaming I/O, snapshots
- **v0.6.0** - Event system, command pattern
- **v0.5.0** - Narrative features, timeline
- **v0.4.0** - Label system refinement
- **v0.3.0** - Non-Hausdorff topology
- **v0.2.0** - Serialization
- **v0.1.0** - Initial release

## Contact

For questions, issues, or suggestions, please open an issue on GitHub.

---

**Cebu**: Where topology meets narrative.
