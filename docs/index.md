# Cebu Documentation

Welcome to the Cebu documentation! Cebu is a modern C++17 library for manipulating simplicial complexes with advanced features including spatial indexing, narrative-driven topology, persistent homology, and more.

## Quick Links

- [Getting Started](getting_started.md) - New to Cebu? Start here!
- [API Documentation](api.md) - Comprehensive API reference
- [FAQ](faq.md) - Common questions and troubleshooting

---

## Documentation by Topic

### Getting Started

| Document | Description |
|-----------|-------------|
| [Getting Started](getting_started.md) | Step-by-step tutorial from installation to first complex |
| [Installation](#installation) | Build and install Cebu on various platforms |

### Core API

| Document | Description |
|-----------|-------------|
| [API Reference](api.md) | Complete API documentation for all core features |
| [Architecture](architecture.md) | Design patterns, internal structure, and philosophy |

### Advanced Features

| Document | Description |
|-----------|-------------|
| [Spatial Indexing](spatial_indexing.md) | BVH and Octree for accelerated spatial queries (5-2000x speedup) |
| [Advanced Serialization](api_advanced_serialization.md) | Version control, snapshots, streaming I/O, change tracking |
| [Equivalence Classes](equivalence_classes.md) | Non-Hausdorff topology and simplex gluing |
| [Topology Operations](topology_operations.md) | Euler characteristic, manifold checking, boundaries, connected components |
| [Absurdity System](absurdity_system.md) | Fuzzy interval numbers, stochastic evolution, dynamic topology morphs (Phase 9a/9b) |
| [Narrative Context](phase9c_narrative_context.md) | Story beats, dramatic templates, narrative-driven absurdity (Phase 9c) |
| [Phase 9 Overview](phase9_absurdity_overview.md) | Complete guide to the absurdity system (Phase 9a/9b/9c) |

### Performance & Optimization

| Document | Description |
|-----------|-------------|
| [Performance Guide](performance.md) | Benchmarks, optimization strategies, profiling tips |

### Reference

| Document | Description |
|-----------|-------------|
| [FAQ](faq.md) | Common questions, error solutions, troubleshooting |

---

## Feature Overview

### Core Topology
- **Unified Simplex Model**: Vertices are 0-simplices for clean representation
- **Efficient Operations**: Add, remove, and query simplices with cascade deletion
- **Adjacency Queries**: Fast neighbor and facet detection
- **Topological Invariants**: Euler characteristic, manifold checking, connected components

### Advanced Features

#### Spatial Indexing
- **BVH**: Bounding Volume Hierarchy for general-purpose spatial queries
- **Octree**: 8-way space partitioning for uniformly distributed data
- **Performance**: 100-2000x faster spatial queries
- **Build Strategies**: SAH, Median, Midpoint, HLBVH, Equal Counts

#### Label System
- **Template-based**: Support any copyable type as labels
- **Numeric Queries**: Filter by high/low/range of labels
- **Absurdity Labels**: Interval-valued fuzzy numbers for narrative metrics
- **Predicate Filtering**: Custom filter functions

#### Narrative Features
- **Timeline**: Time bounds and milestones
- **Story Events**: Time-based events modifying labels
- **Narrative Evolution**: Context-driven topology changes

#### Non-Hausdorff Topology
- **Simplex Gluing**: Identify simplices as topologically equivalent
- **Equivalence Classes**: Union-Find data structure for efficient management
- **Independent Labels**: Glued simplices maintain separate labels

#### Refinement System
- **Edge Refinement**: Insert midpoints for adaptive resolution
- **Triangle Refinement**: Subdivide into 4 triangles
- **Coarsening**: Merge simplices to reduce complexity
- **Label Inheritance**: Copy, interpolate, distribute, or custom strategies

#### Event System
- **Publish-Subscribe**: Event-driven architecture
- **Typed Callbacks**: Strongly-typed event handlers
- **Event Batching**: Performance optimization for bulk operations

#### Command Pattern
- **Undo/Redo**: Full command history with navigation
- **Command Types**: Add, Remove, SetLabel, ApplyEvent commands
- **History Management**: Size limits and pruning

### Serialization & Persistence

#### Basic Serialization
- **JSON**: Human-readable format for debugging and interchange
- **Binary**: Compact format for production use
- **Compression**: ZLIB support for reduced file sizes

#### Advanced Serialization
- **Change Tracking**: Track modifications with diffs
- **Snapshots**: Compressed snapshots with incremental updates
- **Streaming I/O**: Handle large files with constant memory
- **Version Control**: Git-style branches, commits, merges

---

## By Use Case

### 3D Mesh Processing

Start here: [Getting Started](getting_started.md)

Key features:
- Simplicial complex management
- Refinement for adaptive meshing
- Spatial indexing for fast queries
- Serialization for save/load

### Scientific Visualization

Start here: [Topology Operations](topology_operations.md)

Key features:
- Topological invariants (Euler characteristic)
- Manifold checking
- Connected components
- Label-based visualization

### Narrative Simulation

Start here: [API - Narrative Features](api.md#narrative-features)

Key features:
- Timeline management
- Story events
- Narrative evolution
- Absurdity metrics

### Game Level Topology

Start here: [Spatial Indexing](spatial_indexing.md)

Key features:
- BVH for fast collision detection
- Octree for spatial partitioning
- Dynamic updates with incremental rebuilding
- Label system for game properties

### Computational Topology

Start here: [Equivalence Classes](equivalence_classes.md)

Key features:
- Non-Hausdorff topology
- Simplex gluing
- Topological identifications
- Quotient spaces

---

## Learning Path

### Beginner (New to Cebu)

1. [Getting Started Guide](getting_started.md) - Learn basics
2. [API Documentation](api.md) - Explore core API
3. Build your first complex

### Intermediate (Familiar with basics)

1. [Spatial Indexing](spatial_indexing.md) - Accelerate queries
2. [Topology Operations](topology_operations.md) - Analyze topology
3. [Advanced Serialization](api_advanced_serialization.md) - Save/load efficiently

### Advanced (Power user)

1. [Equivalence Classes](equivalence_classes.md) - Non-Hausdorff topology
2. [Performance Guide](performance.md) - Optimize for speed
3. [Architecture](architecture.md) - Understand internals
4. Custom extensions and integrations

---

## Performance Benchmarks

### Core Operations

| Operation | 1K | 10K | 100K | 1M |
|-----------|-----|------|-------|-----|
| Add Simplex | < 0.001ms | < 0.001ms | < 0.001ms | < 0.001ms |
| Remove Simplex | 0.01ms | 0.01ms | 0.01ms | 0.01ms |

### Spatial Query Speedup

| Complex Size | Linear | BVH | Speedup |
|-------------|--------|------|---------|
| 1,000 | 0.1ms | 0.001ms | 100x |
| 10,000 | 1ms | 0.005ms | 200x |
| 100,000 | 10ms | 0.015ms | 667x |
| 1,000,000 | 100ms | 0.05ms | 2000x |

### Serialization Performance

| Format | Write (100K) | Read (100K) | Size |
|--------|--------------|-------------|------|
| JSON | 50ms | 80ms | 10 MB |
| Binary | 20ms | 30ms | 5 MB |
| Binary + ZLIB | 100ms | 50ms | 2 MB |

See [Performance Guide](performance.md) for detailed benchmarks.

---

## Code Examples

### Basic Usage

```cpp
#include "cebu/simplicial_complex.h"

cebu::SimplicialComplex complex;

// Add vertices
auto v0 = complex.add_simplex({});
auto v1 = complex.add_simplex({});
auto v2 = complex.add_simplex({});

// Add triangle
auto tri = complex.add_simplex({v0, v1, v2});
```

### With Labels

```cpp
#include "cebu/simplicial_complex_labeled.h"

cebu::SimplicialComplexLabeled<float> complex;

// Add with label
auto v0 = complex.add_simplex({}, 10.0f);
auto tri = complex.add_simplex({v0, v1, v2}, 20.0f);

// Query by label
auto high = complex.get_high_labeled_simplices(15.0f);
```

### Spatial Indexing

```cpp
#include "cebu/bvh.h"
#include "cebu/vertex_geometry.h"

cebu::VertexGeometry vg;
// ... set vertex positions ...

cebu::BVHTree tree(cebu::BVHBuildStrategy::SAH);
tree.build(all_simplices, vg, get_vertices);

// Fast queries
auto nearest = tree.nearest_neighbor_query(point, 10);
```

---

## Installation

### Requirements

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.14 or higher
- nlohmann/json (automatically fetched)
- ZLIB (optional, for compression)

### Quick Start

```bash
# Clone and build
git clone https://github.com/yourusername/cebu.git
cd cebu
mkdir build && cd build
cmake ..
cmake --build .

# Run tests
ctest --output-on-failure
```

See [Getting Started Guide](getting_started.md) for detailed installation instructions.

---

## Contributing

We welcome contributions! Please see:
- Code of conduct
- Contribution guidelines
- Reporting bugs
- Feature requests

---

## License

See [LICENSE](../LICENSE) file for details.

---

## Support

### Documentation
- [Getting Started](getting_started.md)
- [API Reference](api.md)
- [FAQ](faq.md)

### Community
- GitHub Issues: Report bugs and request features
- GitHub Discussions: Ask questions and share ideas

### Examples
- [examples/](../examples/) - Working code examples
- [tests/](../tests/) - Test cases demonstrating usage

---

## Version History

- **v0.8.0** - Spatial indexing (BVH, Octree)
- **v0.7.0** - Version control, streaming I/O, snapshots
- **v0.6.0** - Event system, command pattern
- **v0.5.0** - Narrative features, timeline
- **v0.4.0** - Label system refinement
- **v0.3.0** - Non-Hausdorff topology
- **v0.2.0** - Serialization
- **v0.1.0** - Initial release

---

**Cebu**: Where topology meets narrative.

**Documentation Version**: 1.0
**Last Updated**: 2025-03-06
**Cebu Version**: 0.8.0
