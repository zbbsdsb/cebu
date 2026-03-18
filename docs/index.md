# Cebu Documentation

Welcome to the Cebu documentation! Cebu is a modern C++20 library for manipulating simplicial complexes with advanced features including spatial indexing, narrative-driven topology, persistent homology, and more.

## Quick Links

- [Getting Started](getting_started.md) - New to Cebu? Start here!
- [Installation](installation.md) - Build and install Cebu
- [API Documentation](api/api_core.md) - Comprehensive API reference
- [FAQ](utilities/faq.md) - Common questions and troubleshooting

---

## Documentation by Topic

### Getting Started

| Document | Description |
|-----------|-------------|
| [Getting Started](getting_started.md) | Step-by-step tutorial from installation to first complex |
| [Installation](installation.md) | Build and install Cebu on various platforms |
| [Basic Tutorial](tutorials/tutorial_basic.md) | Detailed tutorial for basic usage |
| [Spatial Indexing Tutorial](tutorials/tutorial_spatial.md) | Tutorial for spatial indexing features |
| [Narrative Features Tutorial](tutorials/tutorial_narrative.md) | Tutorial for narrative-driven topology |

### Core Concepts

| Document | Description |
|-----------|-------------|
| [Simplicial Complex Basics](core/simplicial_complex.md) | Fundamentals of simplicial complexes |
| [Topology Operations](core/topology.md) | Topological invariants and operations |
| [Label System](core/labels.md) | Attaching custom data to simplices |
| [Basic Serialization](core/serialization.md) | Saving and loading complexes |

### Advanced Features

| Document | Description |
|-----------|-------------|
| [Spatial Indexing](advanced/spatial_indexing.md) | BVH and Octree for accelerated spatial queries |
| [Non-Hausdorff Topology](advanced/non_hausdorff.md) | Simplex gluing and equivalence classes |
| [Absurdity System](advanced/absurdity_system.md) | Fuzzy interval numbers and stochastic evolution |
| [Narrative Context](advanced/narrative_context.md) | Story beats and narrative-driven topology |
| [Phase 9 Overview](advanced/phase9_absurdity_overview.md) | Complete guide to the absurdity system |

### API Reference

| Document | Description |
|-----------|-------------|
| [Core API](api/api_core.md) | Core library API |
| [Spatial Indexing API](api/api_spatial.md) | Spatial indexing API |
| [Serialization API](api/api_serialization.md) | Serialization API |
| [Narrative API](api/api_narrative.md) | Narrative features API |
| [Absurdity API](api/api_absurdity.md) | Absurdity system API |

### Utilities and Resources

| Document | Description |
|-----------|-------------|
| [Performance Guide](utilities/performance.md) | Benchmarks and optimization strategies |
| [Best Practices](utilities/best_practices.md) | Recommended usage patterns |
| [FAQ](utilities/faq.md) | Common questions and troubleshooting |
| [Troubleshooting](utilities/troubleshooting.md) | Debugging common issues |

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

Start here: [Basic Tutorial](tutorials/tutorial_basic.md)

Key features:
- Simplicial complex management
- Refinement for adaptive meshing
- Spatial indexing for fast queries
- Serialization for save/load

### Scientific Visualization

Start here: [Topology Operations](core/topology.md)

Key features:
- Topological invariants (Euler characteristic)
- Manifold checking
- Connected components
- Label-based visualization

### Narrative Simulation

Start here: [Narrative Features Tutorial](tutorials/tutorial_narrative.md)

Key features:
- Timeline management
- Story events
- Narrative evolution
- Absurdity metrics

### Game Level Topology

Start here: [Spatial Indexing Tutorial](tutorials/tutorial_spatial.md)

Key features:
- BVH for fast collision detection
- Octree for spatial partitioning
- Dynamic updates with incremental rebuilding
- Label system for game properties

### Computational Topology

Start here: [Non-Hausdorff Topology](advanced/non_hausdorff.md)

Key features:
- Non-Hausdorff topology
- Simplex gluing
- Topological identifications
- Quotient spaces

---

## Learning Path

### Beginner (New to Cebu)

1. [Installation](installation.md) - Install Cebu
2. [Basic Tutorial](tutorials/tutorial_basic.md) - Learn basics
3. [Simplicial Complex Basics](core/simplicial_complex.md) - Understand fundamentals
4. Build your first complex

### Intermediate (Familiar with basics)

1. [Spatial Indexing Tutorial](tutorials/tutorial_spatial.md) - Accelerate queries
2. [Topology Operations](core/topology.md) - Analyze topology
3. [Label System](core/labels.md) - Add custom data
4. [Basic Serialization](core/serialization.md) - Save/load efficiently

### Advanced (Power user)

1. [Non-Hausdorff Topology](advanced/non_hausdorff.md) - Explore advanced topology
2. [Absurdity System](advanced/absurdity_system.md) - Use narrative features
3. [Performance Guide](utilities/performance.md) - Optimize for speed
4. [API Reference](api/api_core.md) - Dive into the API

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

See [Performance Guide](utilities/performance.md) for detailed benchmarks.

---

## Code Examples

### Basic Usage

```cpp
#include "cebu/simplicial_complex.h"

cebu::SimplicialComplex complex;

// Add vertices
auto v0 = complex.add_vertex();
auto v1 = complex.add_vertex();
auto v2 = complex.add_vertex();

// Add triangle
auto tri = complex.add_triangle(v0, v1, v2);
```

### With Labels

```cpp
#include "cebu/simplicial_complex_labeled.h"

cebu::SimplicialComplexLabeled<float> complex;

// Add with label
auto v0 = complex.add_vertex(10.0f);
auto v1 = complex.add_vertex(20.0f);
auto v2 = complex.add_vertex(30.0f);
auto tri = complex.add_triangle(v0, v1, v2, 20.0f);

// Query by label
auto high = complex.get_high_labeled_simplices(15.0f);
```

### Spatial Indexing

```cpp
#include "cebu/bvh.h"
#include "cebu/vertex_geometry.h"

cebu::SimplicialComplex sc;
cebu::VertexGeometry vg;

// Add vertices with positions
auto v0 = sc.add_vertex();
auto v1 = sc.add_vertex();
auto v2 = sc.add_vertex();

vg.set_position(v0, {0.0, 0.0, 0.0});
vg.set_position(v1, {1.0, 0.0, 0.0});
vg.set_position(v2, {0.0, 1.0, 0.0});

// Add triangle
sc.add_triangle(v0, v1, v2);

// Build BVH
cebu::BVH bvh(sc, vg);

// Fast queries
cebu::Point query_point = {0.5, 0.5, 0.0};
auto nearest = bvh.nearest_neighbor(query_point);
```

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
- [API Reference](api/api_core.md)
- [FAQ](utilities/faq.md)

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

**Documentation Version**: 2.0
**Last Updated**: 2026-03-18
**Cebu Version**: 0.8.0
