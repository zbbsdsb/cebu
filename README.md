# cebu

A C++ library implementing **dynamic simplicial complexes** for the Cebu Space project.
Cebu Space is a non-Hausdorff, narrative-driven topological structure where geometry depends on absurdity and intent.

## Features

### Current (v0.1.0)
- Dynamic addition/removal of simplices (vertices, edges, faces, etc.)
- Cascade deletion for maintaining complex consistency
- Query simplices by dimension, connectivity, and containment
- Adjacency relationship management
- Comprehensive test suite

### Planned
- Contextual labels (e.g., absurdity, narrative tension)
- Query simplices by label
- Morphism operations to reshape the complex based on rules
- Non-Hausdorff topology support (gluing operations)
- Event system for external integration
- Serialization/deserialization

## Documentation

- [API Reference](docs/api.md) - Detailed API documentation
- [Architecture](docs/architecture.md) - Design and implementation details

## Quick Start

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
