# Cebu Project Structure

## 1. Project Overview

Cebu is a modern C++20 library for manipulating simplicial complexes with advanced features including spatial indexing, narrative-driven topology, persistent homology, and an enhanced absurdity system.

## 2. Directory Structure

```
cebu/
├── .github/             # GitHub configuration files
│   ├── ISSUE_TEMPLATE/   # Issue templates
│   ├── workflows/        # GitHub Actions workflows
│   ├── CODEOWNERS        # Code ownership definitions
│   ├── CODE_OF_CONDUCT.md # Code of conduct
│   ├── FUNDING.yml       # Funding configuration
│   ├── PULL_REQUEST_TEMPLATE.md # PR template
│   ├── SECURITY.md       # Security policy
│   └── auto-assign.yml   # PR auto-assignment configuration
├── .trae/               # Trae IDE configuration and documents
│   └── documents/        # Project planning documents
├── assets/              # Visual assets
│   ├── cebu_logo.png     # Project logo (PNG)
│   └── cebu_logo.svg     # Project logo (SVG)
├── benchmarks/          # Performance benchmarks
│   └── README.md         # Benchmark documentation
├── cmake/               # CMake configuration files
│   └── cebuConfig.cmake.in # CMake package configuration
├── dependencies/        # External dependencies
│   └── json-3.11.2/      # nlohmann/json library
├── docs/                # Documentation
│   ├── api/             # API documentation
│   ├── core/            # Core concepts
│   ├── advanced/        # Advanced features
│   ├── tutorials/       # Tutorials
│   ├── utilities/       # Utility guides
│   ├── index.md         # Documentation landing page
│   ├── installation.md  # Installation guide
│   └── getting_started.md # Getting started guide
├── examples/            # Example code
├── include/             # Public headers
│   └── cebu/            # Cebu namespace headers
├── src/                 # Source code
├── tests/               # Test files
├── .clang-format        # Clang-format configuration
├── .gitignore           # Git ignore file
├── CHANGELOG.md         # Version history
├── CMakeLists.txt       # CMake build configuration
├── CODE_OF_CONDUCT.md   # Code of conduct
├── CONTRIBUTING.md      # Contribution guidelines
├── DCO                  # Developer Certificate of Origin
├── LICENSE              # MIT License
├── NOTICE               # Third-party notices
├── README.md            # Project README
└── SECURITY.md          # Security policy
```

## 3. Module Architecture

### 3.1 Core Modules

| Module | Description | Files | Location |
|--------|-------------|-------|----------|
| **simplicial_complex** | Core simplicial complex operations | simplex.cpp, simplicial_complex.cpp | src/ |
| **topology** | Topological invariants and operations | topology.cpp | src/ |
| **labels** | Label system for simplices | labels.cpp | src/ |
| **serialization** | JSON and binary serialization | json_serialization.cpp, binary_serialization.cpp | src/ |
| **compression** | ZLIB compression support | compression.cpp | src/ |

### 3.2 Advanced Modules

| Module | Description | Files | Location |
|--------|-------------|-------|----------|
| **spatial** | Spatial indexing (BVH, Octree) | bvh.cpp, octree.cpp, vertex_geometry.cpp | src/ |
| **narrative** | Narrative-driven topology | narrative_context.cpp, story_events.cpp | src/ |
| **absurdity** | Absurdity system | absurdity.cpp, absurdity_engine.cpp | src/ |
| **non_hausdorff** | Non-Hausdorff topology | simplicial_complex_non_hausdorff.cpp, equivalence_classes.cpp | src/ |
| **refinement** | Mesh refinement and coarsening | refinement.cpp | src/ |

### 3.3 Utility Modules

| Module | Description | Files | Location |
|--------|-------------|-------|----------|
| **event_system** | Event-driven architecture | event_system.cpp | src/ |
| **command** | Command pattern with undo/redo | command.cpp | src/ |
| **version_control** | Git-style versioning | version_control.cpp, change_tracker.cpp, snapshot_manager.cpp | src/ |
| **streaming** | Streaming I/O for large files | streaming_io.cpp | src/ |
| **parallel** | Parallel execution | parallel_executor.cpp | src/ |
| **profiler** | Performance profiling | profiler.cpp | src/ |

## 4. Build System

### 4.1 CMake Configuration

The project uses CMake 3.14+ for build configuration. Key CMake options:

- `CEBU_BUILD_TESTS` - Build test suite (OFF by default)
- `CEBU_BUILD_EXAMPLES` - Build examples (OFF by default)
- `CEBU_WITH_ZLIB` - Enable ZLIB compression support (ON by default)

### 4.2 Build Targets

| Target | Description |
|--------|-------------|
| `cebu_core` | Core library |
| `test_*` | Test executables |
| `phase*_demo` | Example executables |
| `cebu_benchmark` | Performance benchmark |

### 4.3 Dependencies

| Dependency | Purpose | Location |
|------------|---------|----------|
| nlohmann/json | JSON serialization | dependencies/json-3.11.2/ |
| ZLIB | Compression (optional) | System library |

## 5. API Architecture

### 5.1 Core API

| Class | Description | Header |
|-------|-------------|--------|
| `SimplicialComplex` | Core simplicial complex class | include/cebu/simplicial_complex.h |
| `Simplex` | Simplex representation | include/cebu/simplex.h |
| `SimplicialComplexLabeled<T>` | Labeled simplicial complex | include/cebu/simplicial_complex_labeled.h |
| `Topology` | Topological operations | include/cebu/topology.h |

### 5.2 Spatial API

| Class | Description | Header |
|-------|-------------|--------|
| `BVHTree` | Bounding Volume Hierarchy | include/cebu/bvh.h |
| `Octree` | Octree spatial index | include/cebu/octree.h |
| `VertexGeometry` | Vertex position management | include/cebu/vertex_geometry.h |

### 5.3 Narrative API

| Class | Description | Header |
|-------|-------------|--------|
| `Timeline` | Time management | include/cebu/timeline.h |
| `StoryEvent` | Narrative events | include/cebu/story_event.h |
| `NarrativeContext` | Narrative management | include/cebu/narrative_context.h |

### 5.4 Absurdity API

| Class | Description | Header |
|-------|-------------|--------|
| `FuzzyInterval` | Fuzzy interval numbers | include/cebu/absurdity.h |
| `AbsurditySource` | Absurdity data source | include/cebu/absurdity.h |
| `StochasticEvolution` | Stochastic evolution system | include/cebu/absurdity.h |
| `AbsurdityEngine` | Absurdity system manager | include/cebu/absurdity_engine.h |

### 5.5 Serialization API

| Class | Description | Header |
|-------|-------------|--------|
| `JsonSerializer` | JSON serialization | include/cebu/json_serialization.h |
| `BinarySerializer` | Binary serialization | include/cebu/binary_serialization.h |
| `SnapshotManager` | Snapshot management | include/cebu/snapshot_manager.h |
| `ChangeTracker` | Change tracking | include/cebu/change_tracker.h |

## 6. Data Flow

### 6.1 Core Data Flow

```
┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐
│ Simplicial      │     │ Simplex         │     │ Topology        │
│ Complex         │────>│ Operations      │────>│ Invariants      │
└─────────────────┘     └─────────────────┘     └─────────────────┘
        │                      │                      │
        ▼                      ▼                      ▼
┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐
│ Label System    │     │ Serialization  │     │ Event System    │
└─────────────────┘     └─────────────────┘     └─────────────────┘
```

### 6.2 Advanced Data Flow

```
┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐
│ Spatial Indexing│     │ Narrative      │     │ Absurdity       │
│ (BVH/Octree)    │────>│ Features       │────>│ System          │
└─────────────────┘     └─────────────────┘     └─────────────────┘
        │                      │                      │
        ▼                      ▼                      ▼
┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐
│ Non-Hausdorff   │     │ Refinement     │     │ Version Control │
│ Topology        │     │ System         │     │                 │
└─────────────────┘     └─────────────────┘     └─────────────────┘
```

## 7. Design Patterns

### 7.1 Creational Patterns

- **Factory Method**: Used for creating simplex instances
- **Builder Pattern**: Used for complex construction (e.g., BVH build strategies)
- **Singleton**: Used for global managers (e.g., AbsurdityEngine)

### 7.2 Structural Patterns

- **Composite Pattern**: Used for simplicial complexes (simplices contain other simplices)
- **Decorator Pattern**: Used for labeled simplicial complexes
- **Adapter Pattern**: Used for serialization adapters

### 7.3 Behavioral Patterns

- **Observer Pattern**: Used for event system
- **Command Pattern**: Used for undo/redo functionality
- **Strategy Pattern**: Used for BVH build strategies
- **Visitor Pattern**: Used for topological operations

## 8. Key Interfaces

### 8.1 Core Interfaces

| Interface | Description | Methods |
|-----------|-------------|---------|
| `SimplexInterface` | Base simplex operations | `id()`, `dimension()`, `faces()` |
| `ComplexInterface` | Base complex operations | `add_simplex()`, `remove_simplex()`, `has_simplex()` |
| `LabelInterface<T>` | Label operations | `get_label()`, `set_label()`, `filter_by_label()` |

### 8.2 Spatial Interfaces

| Interface | Description | Methods |
|-----------|-------------|---------|
| `SpatialIndex` | Spatial index operations | `build()`, `query()`, `nearest_neighbor()` |
| `GeometryProvider` | Geometry data provider | `get_position()`, `get_bounds()` |

### 8.3 Serialization Interfaces

| Interface | Description | Methods |
|-----------|-------------|---------|
| `Serializer` | Serialization operations | `serialize()`, `deserialize()` |
| `Compressor` | Compression operations | `compress()`, `decompress()` |

## 9. Performance Considerations

### 9.1 Time Complexity

| Operation | Complexity | Notes |
|-----------|------------|-------|
| Add simplex | O(1) | Amortized |
| Remove simplex | O(k) | k = number of cofaces |
| Spatial query (BVH) | O(log n) | n = number of simplices |
| Spatial query (linear) | O(n) | n = number of simplices |

### 9.2 Memory Usage

| Component | Memory Usage | Notes |
|-----------|-------------|-------|
| Simplex | ~24 bytes | Varies with dimension |
| BVH Node | ~48 bytes | Includes bounds and child pointers |
| Octree Node | ~32 bytes | Includes bounds and child pointers |

## 10. Extensibility

### 10.1 Extension Points

- **Label Types**: Template-based label system
- **Spatial Indexes**: Extensible spatial index interface
- **Build Strategies**: Custom BVH build strategies
- **Serialization Formats**: Extensible serializer interface
- **Event Handlers**: Custom event listeners

### 10.2 Plugin Architecture

The library uses a modular design that allows for easy extension:
- Header-only components where possible
- Clear separation of interfaces and implementations
- Dependency injection for extensibility
- Template-based customization

## 11. Testing Strategy

### 11.1 Test Types

- **Unit Tests**: Test individual components
- **Integration Tests**: Test component interactions
- **Performance Tests**: Test performance benchmarks
- **Regression Tests**: Test for bug fixes

### 11.2 Test Coverage

- Core functionality: > 90%
- Advanced features: > 80%
- Utility modules: > 70%

## 12. Deployment

### 12.1 Package Formats

- **CMake Package**: For CMake-based projects
- **vcpkg**: For vcpkg package manager
- **Conan**: For Conan package manager
- **Direct Download**: Precompiled binaries

### 12.2 Supported Platforms

- **Windows**: Visual Studio 2019+
- **Linux**: GCC 10+, Clang 10+
- **macOS**: Clang 10+

## 13. Conclusion

The Cebu project follows a modular, extensible architecture that provides a solid foundation for simplicial complex operations with advanced features. The clear separation of concerns, well-defined interfaces, and comprehensive documentation make it easy for developers to understand and extend the library.

---

**Document Created**: 2026-03-28
**Last Updated**: 2026-03-28
**Maintainer**: CeaserZhao (Oasis Company)