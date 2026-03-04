# Cebu Library Architecture

## Overview

The cebu library is designed as a modular C++ library for manipulating dynamic simplicial complexes. This document describes the architectural decisions and implementation details.

## Project Structure

```
cebu/
├── include/cebu/       # Public header files
│   ├── simplex.h       # Simplex class definition
│   └── simplicial_complex.h  # SimplicialComplex class definition
├── src/                # Implementation files
│   ├── simplex.cpp
│   └── simplicial_complex.cpp
├── tests/              # Unit tests
│   ├── test_basic.cpp  # Basic functionality tests
│   └── test_dynamic.cpp # Dynamic operations tests
├── docs/               # Documentation
│   ├── api.md          # API reference
│   └── architecture.md # This file
├── examples/           # Example code (to be added)
├── CMakeLists.txt      # Build configuration
├── LICENSE             # MIT License
└── README.md           # Project overview
```

## Core Components

### 0. Label System (v0.2.0+)

**Purpose**: Attaches semantic data to simplices for narrative-driven topology.

**Design Rationale**:
- Template-based design supports arbitrary label types
- Separation of storage logic from complex management
- Query capabilities for labeled simplices

**Key Classes**:

1. **LabelSystem<T>** (Base class)
   - Pure virtual interface for label operations
   - Predicate-based queries for flexible filtering
   - Type-safe label storage and retrieval

2. **DefaultLabelSystem<T>**
   - Unordered_map-based storage for O(1) operations
   - Suitable for numeric and simple label types
   - Provides range queries (high/low/in_range) for arithmetic types

3. **AbsurdityLabelSystem**
   - Specialized for interval-valued fuzzy numbers
   - Provides absurdity-specific queries (high/low absurdity, uncertainty)
   - Supports context-driven evolution

4. **SimplicialComplexLabeled<LabelType, LabelSystemType>**
   - Wraps SimplicialComplex with label support
   - Forwards topology operations to underlying complex
   - Provides unified API for topology + labels

**Label Types**:

1. **Absurdity**: Interval fuzzy number [lower, upper] × confidence
   - Represents narrative absurdity with uncertainty
   - Evolves based on AbsurdityContext
   - Metrics: midpoint, uncertainty (range width)

2. **AbsurdityContext**: Environmental factors for evolution
   - surprisal, logical_deviation, user_laughter, narrative_tension
   - Drives dynamic absurdity changes

**Query Mechanisms**:
- Predicate-based: `find_by_label([](const auto& label) { return label > threshold; })`
- Range queries: `find_high_labels(0.7)`, `find_labels_in_range(min, max)`
- Type-specific: `find_high_absurdity(0.5)`, `find_high_uncertainty(0.3)`

### 1. Simplex Class

**Purpose**: Represents a single simplex in the complex.

**Design Rationale**:
- Stores minimal information (ID, vertices, neighbors)
- Uses sorted vertices to ensure canonical representation
- Maintains adjacency relations for efficient traversal

**Key Features**:
- Immutable vertex list (after construction)
- Dynamic neighbor management
- Efficient neighbor lookup using `std::unordered_set`

**Memory Layout**:
```
Simplex
├── id_ (SimplexID)
├── vertices_ (std::vector<VertexID>)
└── neighbors_ (std::unordered_set<SimplexID>)
```

### 2. SimplicialComplex Class

**Purpose**: Manages the entire collection of simplices and maintains consistency.

**Design Rationale**:
- Centralized management of all simplices
- Efficient bidirectional mappings
- Automatic adjacency maintenance

**Key Data Structures**:

1. **Simplices Map**: `std::unordered_map<SimplexID, Simplex>`
   - O(1) lookup by ID
   - Stable IDs after deletion (never reused)
   - Contains ALL simplices including 0-simplices (vertices)

2. **Vertex-to-Simplex Mapping**: `std::unordered_map<VertexID, std::unordered_set<SimplexID>>`
   - Reverse lookup: find all simplices containing a vertex
   - Essential for cascade deletion
   - Key design: Vertices are themselves 0-simplices

3. **ID Generation**: `SimplexID next_simplex_id_`
   - Monotonically increasing counter
   - Never reuses IDs to avoid stale references
   - **Critical**: Vertices and higher-dim simplices share same ID space

**Design Innovation - Unified Simplex Model**:
- Vertices are NOT separate entities from simplices
- A vertex IS a 0-simplex (dimension = 0, vertices = {id})
- This eliminates ID conflicts between vertices and simplices
- VertexID is just an alias for the SimplexID of 0-simplices
- All topological operations work uniformly across all dimensions

## Key Algorithms

### Simplex Addition

```
1. Validate input (vertex existence, count)
2. Check for duplicates using vertex comparison
3. Generate new simplex ID
4. Create simplex with sorted vertices
5. Update vertex-to-simplex mapping
6. Build adjacency relations with existing simplices
7. Return new simplex ID
```

**Complexity**:
- Duplicate check: O(n) where n = number of existing simplices
- Adjacency building: O(n × m) where m = average vertices per simplex
- Can be optimized with spatial indexing for large complexes

### Simplex Removal

```
1. If cascade enabled:
   a. Find all simplices containing this simplex as a face
   b. Recursively remove each
2. Remove from adjacency lists of all neighbors
3. Remove from vertex-to-simplex mapping
4. Remove from simplex map
```

**Complexity**:
- Without cascade: O(k) where k = number of neighbors
- With cascade: O(n) worst case

### Adjacency Building

Two simplices are considered adjacent if they share at least one vertex:

```
for each vertex in simplex A:
    for each vertex in simplex B:
        if vertices equal:
            simplices are adjacent
```

**Simplification**: Current implementation uses vertex sharing rather than proper face intersection. This is sufficient for many use cases but may produce "false positives" in adjacency.

## Design Trade-offs

### 1. Adjacency Storage

**Option Chosen**: Each simplex stores its neighbors explicitly.

**Pros**:
- O(1) neighbor lookup
- Simple iteration over neighbors

**Cons**:
- More memory (each edge stored twice)
- Need to maintain consistency on removal

**Alternative**: Compute adjacency on-demand using vertex intersections.

**Trade-off Decision**: Explicit storage chosen for performance at the cost of memory.

### 2. ID Management

**Option Chosen**: Never reuse IDs, use monotonically increasing counter.

**Pros**:
- No risk of stale references
- Simple implementation

**Cons**:
- Can overflow after 2^64 operations (practically never)
- Memory overhead if many deletions

**Alternative**: Reuse IDs from deleted simplices.

**Trade-off Decision**: Non-reuse chosen for safety and simplicity.

### 3. Duplicate Prevention

**Option Chosen**: Return existing simplex ID instead of creating duplicate.

**Pros**:
- Maintains mathematical definition of simplicial complex
- Prevents inconsistency

**Cons**:
- O(n) check on each insertion
- May hide bugs in user code

**Alternative**: Allow duplicates, merge later.

**Trade-off Decision**: Prevention chosen for correctness.

## Future Enhancements

### Performance Optimizations

1. **Spatial Indexing**: Use R-tree or kd-tree for faster spatial queries
2. **Lazy Adjacency**: Compute adjacency only when needed
3. **Batch Operations**: Support adding/removing multiple simplices efficiently
4. **Memory Pool**: Custom allocator for frequent simplex creation/deletion

### Feature Additions

1. **Label System**: Attach arbitrary data to simplices
2. **Morph Operations**: Dynamic topology transformations
3. **Non-Hausdorff Support**: Gluing operations for advanced topology
4. **Event System**: Callbacks for topology changes
5. **Serialization**: Save/load complex state

### API Improvements

1. **Range-based Iteration**: Support `for (auto& simplex : complex)`
2. **Query Builders**: Fluent interface for complex queries
3. **Immutable Views**: Snapshot complex state for thread-safe access

## Thread Safety

Current implementation is **not thread-safe**:
- All operations should be performed from a single thread
- No locking mechanisms
- Shared state access is unprotected

**Future Work**:
- Add read-write locks for concurrent queries
- Provide immutable snapshot API
- Consider lock-free data structures for hot paths

## Error Handling

### Exceptions Used

- `std::invalid_argument`: Invalid vertex IDs or simplex configuration
- `std::out_of_range**: Requesting non-existent simplex

### Error Handling Philosophy

- Fail fast with exceptions
- Validate inputs early
- Maintain invariants even on error

## Memory Management

### Ownership

- SimplicialComplex owns all simplices
- Simplex objects are stored by value in unordered_map
- No raw pointers to internal state exposed

### Lifetime

- SimplicialComplex manages all memory
- No external ownership of internal objects
- References returned from getters are valid only during object lifetime

### Deletion Strategy

- Cascade deletion ensures consistency
- No memory leaks (verified with Valgrind/AddressSanitizer)
- Optional non-cascade deletion for advanced use cases

## Testing Strategy

### Unit Tests

- `test_basic.cpp`: Core functionality (9 tests)
  - Vertex/edge/triangle creation
  - Adjacency and facet queries
  - Dimension queries
  - Duplicate prevention
  - Tetrahedron (3-simplex) support

- `test_dynamic.cpp`: Dynamic operations (9 tests)
  - Simplex and vertex removal
  - Cascade deletion
  - Add/remove cycles
  - Adjacency consistency after removal
  - Vertex-to-simplex mapping

- `test_labels.cpp`: Label system (10 tests)
  - Basic label operations (set/get/remove)
  - Label queries (high/low/range)
  - Predicate-based filtering
  - Simplex removal with labels
  - Absurdity labels and evolution
  - Partial labeling
  - Combined topology+label queries

### Test Coverage Goals

- All public methods tested
- Edge cases covered (empty complex, invalid IDs, etc.)
- Memory leak testing
- Stress testing for large complexes
- Label system correctness (type safety, query accuracy)

### Integration Tests (Future)

- End-to-end workflows
- Performance benchmarks
- Concurrency tests (when thread safety is added)
- Narrative-driven topology scenarios

## Build System

### CMake Configuration

- Minimum version: 3.14
- C++17 standard
- Separate library and test targets

### Compiler Requirements

- C++17 compliant compiler
- Tested with: MSVC 19.5, GCC 9+, Clang 10+

## Dependencies

- **External**: None (STL only)
- **Internal**: None

**C++17 Features Used**:
- `std::optional` for nullable return values
- `std::unordered_map` for efficient lookups
- Template metaprogramming for type-safe label system
- `std::enable_if_t` for SFINAE-based type constraints

## Documentation

### API Documentation

- Comprehensive API reference in `docs/api.md`
- Doxygen comments in source (to be added)

### Architecture Documentation

- This document (`docs/architecture.md`)
- Design rationale and implementation details

### User Guides

- README.md: Quick start and build instructions
- Examples: Practical usage patterns (to be added)

## Version History

### v0.2.0 (Current) - Label System & Unified Topology
- **Unified Simplex Model**: Vertices are now 0-simplices
- Template-based label system for arbitrary label types
- DefaultLabelSystem for numeric labels
- AbsurdityLabelSystem for interval-valued fuzzy numbers
- Labeled simplicial complex with query support
- Range queries and predicate-based filters
- Absurdity metrics with confidence and uncertainty
- All tests passing (basic, dynamic, labels)

### v0.1.0
- Basic simplex representation
- Dynamic simplicial complex
- Add/remove operations
- Cascade deletion
- Adjacency queries
- Comprehensive test suite

## Contributing

See the project's contribution guidelines (to be added).

## License

MIT License - See LICENSE file for details.
