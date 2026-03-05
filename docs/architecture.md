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

### 0.1. Command System (v0.3.1+)

**Purpose**: Enables undo/redo functionality for all reversible operations.

**Design Rationale**:
- Command pattern for reversible operations
- State capture for accurate undo
- History management with size limits
- Template-based for flexibility across complex types

**Key Classes**:

1. **Command** (Base class)
   - Abstract interface for all commands
   - Execute/Undo methods
   - Description for debugging

2. **Concrete Commands**
   - AddSimplexCommand: Stores vertices, removes simplex on undo
   - RemoveSimplexCommand: Stores simplex data for recreation
   - SetLabelCommand: Stores old label for restoration
   - ApplyEventCommand: Stores old labels of affected simplices

3. **CommandHistory**
   - Manages command execution stack
   - Supports undo/redo navigation
   - Size limits to control memory usage
   - Truncates future commands on new execution

**Data Flow**:
```
User Action → Command::execute() → History.push()
             ↓
         State Change
         ↓
         History.position++

Undo → Command::undo() → State Restore → History.position--
Redo → Command::execute() → State Change → History.position++
```

**Memory Management**:
- Commands capture minimal state needed for undo
- Max size prevents unbounded memory growth
- Unique_ptr ensures proper ownership

### 0.2. Serialization System (v0.3.1+)

**Purpose**: Enables saving and loading complex states for persistence.

**Design Rationale**:
- JSON format for human-readable storage
- Binary format for efficient storage
- Template-based for all complex types
- Versioning support for future compatibility

**Key Classes**:

1. **JsonSerializer**
   - Serialize basic complexes (simplices only)
   - Serialize labeled complexes (includes labels)
   - Serialize narrative complexes (includes timeline, events)
   - Hierarchical JSON structure

2. **BinarySerializer**
   - Compact binary format
   - Magic number and versioning
   - Efficient deserialization
   - Direct binary read/write

**JSON Format**:
```json
{
  "version": "1.0",
  "type": "narrative",
  "simplices": [...],
  "labels": {...},
  "timeline": {...},
  "events": [...],
  "current_time": 50.0
}
```

**Binary Format**:
- Header: 4-byte magic + 4-byte version
- Simplex count: 8 bytes
- Per-simplex: ID (8) + vertex count (8) + vertices (4 each)

### 0. Narrative Layer (v0.3.0+)

**Purpose**: Enables timeline-driven narrative evolution of simplicial complexes.

**Design Rationale**:
- Separation of concerns: timeline, events, and complex management
- Event-driven architecture for flexible narrative composition
- Integration with label system for absurdity evolution

**Key Classes**:

1. **StoryEvent**
   - Represents a single narrative event
   - Contains timestamp, description, and affected simplices
   - Carries AbsurdityContext for label evolution

2. **StoryEventSystem**
   - Manages collection of story events
   - Time-range queries for event retrieval
   - Validations for timestamps and event operations

3. **Timeline**
   - Manages narrative time bounds [start, end]
   - Supports milestones at specific times
   - Navigation methods (next/previous milestone)
   - Validation for event timestamps

4. **TopologyOperations**
   - Static utility class for topological operations
   - Vertex gluing (identification)
   - Batch vertex gluing
   - Simplex boundary gluing
   - Boundary computation
   - Connected components analysis
   - Dimension filtering
   - Euler characteristic computation
   - Manifold checking

5. **SimplicialComplexNarrative<LabelType>**
   - Extends SimplicialComplexLabeled with narrative support
   - Integrates timeline and event system
   - Evolves labels based on events through timeline
   - Template-based for flexible label types

**Data Flow**:
```
Timeline → Events → Simplices → Labels
   ↓           ↓           ↓          ↓
Milestones  Context    Selection   Evolution
```

**Evolution Process**:
1. User calls `evolve_to(timestamp)`
2. Timeline validates timestamp
3. Event system returns events in range [start, timestamp]
4. For each event, apply to affected simplices
5. Labels evolve based on AbsurdityContext
6. Current time updates to timestamp

### 1. Label System (v0.2.0+)

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

- `test_story_events.cpp`: Story event system (9 tests)
  - Add/retrieve events
  - Time-range queries
  - Event removal and clearing
  - Invalid input handling
  - Multiple events

- `test_timeline.cpp`: Timeline management (12 tests)
  - Timeline bounds and duration
  - Milestone management
  - Time containment
  - Navigation (next/previous milestone)
  - Invalid input handling

- `test_topology_ops.cpp`: Topological operations (9 tests)
  - Vertex gluing
  - Boundary computation
  - Dimension filtering
  - Connected components
  - Error handling

- `test_narrative.cpp`: Narrative complex integration (8 tests)
  - Basic narrative complex operations
  - Event addition and application
  - Timeline evolution
  - Bounds validation
  - Multi-component scenarios
  - Milestone integration

- `test_topology_ops_advanced.cpp`: Advanced topology operations (13 tests)
  - Simplex boundary gluing
  - Batch vertex gluing
  - Euler characteristic computation
  - Manifold checking
  - Error handling

- `test_command_history.cpp`: Command system (14 tests)
  - AddSimplexCommand
  - RemoveSimplexCommand
  - SetLabelCommand
  - CommandHistory basic operations
  - Undo/redo functionality
  - Multiple undo/redo
  - History size limits
  - Error cases

- `test_serialization.cpp`: Serialization system (11 tests)
  - JSON serialization (basic, labeled, absurdity, narrative)
  - Binary serialization
  - Round-trip testing
  - Error handling
  - Format validity

### Test Coverage Goals

- All public methods tested
- Edge cases covered (empty complex, invalid IDs, etc.)
- Memory leak testing
- Stress testing for large complexes
- Label system correctness (type safety, query accuracy)
- Timeline consistency and validation
- Narrative evolution correctness

### Integration Tests (Future)

- End-to-end workflows
- Performance benchmarks
- Concurrency tests (when thread safety is added)
- Complex narrative scenarios with multiple events
- Absurdity-driven topological transformations

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

### v0.3.1 (Current) - Advanced Narrative Features
- **Advanced Topology Operations**:
  - Simplex boundary gluing (glue_simplices_by_boundary)
  - Batch vertex gluing (batch_glue_vertices)
  - Euler characteristic computation (compute_euler_characteristic)
  - Manifold checking (is_manifold)
- **Command System**: Undo/redo support for reversible operations
  - Command base class and concrete implementations
  - CommandHistory manager with size limits
  - Support for AddSimplex, RemoveSimplex, SetLabel, ApplyEvent commands
- **Serialization System**:
  - JSON serialization for basic, labeled, and narrative complexes
  - Binary serialization for efficient storage
  - Round-trip serialization/deserialization
- **Enhanced Narrative Complex**:
  - Integrated CommandHistory for undo/redo
  - State serialization methods
  - Snapshot creation and restoration
  - Helper methods: undo(), redo(), can_undo(), can_redo()
- All tests passing (basic, dynamic, labels, story_events, timeline, topology_ops, narrative, topology_ops_advanced, serialization)

### v0.3.0 - Narrative-Driven Topology
- **Story Event System**: Timeline-based narrative events affecting simplices
- **Timeline Management**: Time bounds, milestones, and navigation
- **Topology Operations**: Vertex gluing, boundary computation, connected components
- **Narrative Complex**: Integration of events, timeline, and label evolution
- **Absurdity Evolution**: Dynamic label changes based on narrative context
- All tests passing (basic, dynamic, labels, story_events, timeline, topology_ops, narrative)

### v0.2.0 - Label System & Unified Topology
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
