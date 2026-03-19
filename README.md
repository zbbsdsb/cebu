<div align="center">

![Cebu Logo](assets/cebu_logo.png)

# Cebu: Advanced Simplicial Complex Library

[![GitHub release](https://img.shields.io/github/release/ceaserzhao/cebu.svg?style=flat-square)](https://github.com/ceaserzhao/cebu/releases)
[![GitHub license](https://img.shields.io/badge/license-MIT-blue.svg?style=flat-square)](https://github.com/ceaserzhao/cebu/blob/main/LICENSE)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg?style=flat-square)](https://en.cppreference.com/w/cpp/20)
[![CMake](https://img.shields.io/badge/CMake-3.14%2B-blue.svg?style=flat-square)](https://cmake.org/)
[![Build Status](https://img.shields.io/github/actions/workflow/status/ceaserzhao/cebu/ci.yml?branch=main&style=flat-square)](https://github.com/ceaserzhao/cebu/actions)
[![GitHub issues](https://img.shields.io/github/issues/ceaserzhao/cebu.svg?style=flat-square)](https://github.com/ceaserzhao/cebu/issues)
[![GitHub stars](https://img.shields.io/github/stars/ceaserzhao/cebu.svg?style=flat-square)](https://github.com/ceaserzhao/cebu/stargazers)
[![Contributor Covenant](https://img.shields.io/badge/Contributor%20Covenant-2.0-4baaaa.svg?style=flat-square)](CODE_OF_CONDUCT.md)

</div>

**Cebu** is a modern C++20 library for manipulating simplicial complexes with advanced features including spatial indexing, narrative-driven topology, persistent homology, and an enhanced absurdity system.

---

## ✨ Features

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

### Enhanced Absurdity System
- **Fuzzy Interval Numbers**: Represent uncertainty with confidence factors
- **Stochastic Evolution**: Simulate absurdity dynamics over time
- **Multi-Source Fusion**: Combine information from multiple sources
- **Uncertainty Comparison**: Compare fuzzy values with uncertainty awareness
- **Absurdity Field**: Spatial distribution of absurdity across simplices
- **Absurdity Engine**: Manage the entire absurdity system

### Serialization & Persistence
- **JSON Serialization**: Human-readable format
- **Binary Serialization**: Efficient compact format
- **Streaming I/O**: Handle large files without loading entirely into memory
- **Compression**: ZLIB compression support
- **Snapshots**: Compressed snapshots with incremental updates

## Quick Start

### Installation

#### Method A: CMake (Recommended)

```bash
# Clone the repository
git clone https://github.com/yourusername/cebu.git
cd cebu

# Build with CMake
mkdir build && cd build
cmake ..
cmake --build . --config Release

# Install (optional)
cmake --install .

# Run tests
ctest --output-on-failure
```

#### Method B: vcpkg (Coming Soon)

```bash
# Install via vcpkg
vcpkg install cebu
```

#### Method C: Conan (Coming Soon)

```bash
# Install via Conan
conan install --requires=cebu/1.0.0
```

#### Method D: Direct Download

Download the precompiled binaries from the [GitHub Releases](https://github.com/yourusername/cebu/releases) page.

### 💻 Basic Usage

#### Using CMake find_package()

Once Cebu is installed, you can use it in your CMake project with `find_package()`:

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.14)
project(my_project)

find_package(cebu CONFIG REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app cebu::cebu_core)
```

```cpp
// main.cpp
#include <cebu/simplicial_complex.h>

int main() {
    cebu::SimplicialComplex sc;
    auto v1 = sc.add_vertex();
    auto v2 = sc.add_vertex();
    sc.add_edge(v1, v2);
    return 0;
}
```

#### Direct Usage

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

### Using Absurdity System

```cpp
#include "cebu/absurdity.h"

int main() {
    using namespace cebu;

    // Create fuzzy intervals
    FuzzyInterval a(0.3, 0.5, 0.9);  // [0.3, 0.5] with 90% confidence
    FuzzyInterval b(0.4, 0.7, 0.8);  // [0.4, 0.7] with 80% confidence

    // Arithmetic operations
    FuzzyInterval sum = a + b;
    std::cout << "Sum: " << sum.to_string() << std::endl;

    // Stochastic evolution
    EvolutionParams params;
    params.decay_rate = 0.95;
    params.volatility = 0.1;
    
    StochasticEvolution evolution(params);
    auto trajectory = evolution.evolve(a, 10);

    // Multi-source fusion
    std::vector<AbsurditySource> sources = {
        {"Source A", FuzzyInterval(0.3, 0.5, 0.9), 0.9, 1.0},
        {"Source B", FuzzyInterval(0.4, 0.6, 0.8), 0.7, 0.8}
    };
    
    FuzzyInterval fused = FusionStrategy::fuse(sources);
    std::cout << "Fused: " << fused.to_string() << std::endl;

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

    // Get all simplices
    std::vector<cebu::SimplexID> all_simplices = {
        vg.get_vertex(0), vg.get_vertex(1), vg.get_vertex(2)
    };

    // Build BVH spatial index
    cebu::BVHTree tree(cebu::BVHBuildStrategy::SAH);
    tree.build(all_simplices, vg, [&](cebu::SimplexID sid) {
        return std::vector<cebu::Point3D>{vg.get_vertex(sid)};
    });

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
- [Absurdity System Guide](docs/absurdity_system.md) - Enhanced absurdity system usage
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

- C++20 compatible compiler (GCC 10+, Clang 10+, MSVC 2019+)
- CMake 3.14 or higher
- nlohmann/json (automatically fetched by CMake)
- ZLIB (optional, for compression)

## License

MIT License. See [LICENSE](LICENSE) file for details.

### Third-Party Licenses

This project includes the following third-party software:
- **nlohmann/json** - MIT License
- **ZLIB** - ZLIB License (optional)

See [NOTICE](NOTICE) file for full details.

## Legal Disclaimer

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

## Trademark Notice

The Cebu name and logo are trademarks of CeaserZhao (Oasis Company). The MIT License does not grant permission to use the trade names, trademarks, service marks, or product names of the Licensor, except as required for reasonable and customary use in describing the origin of the Work.

## Security

For security issues, please see our [Security Policy](SECURITY.md).

## Code of Conduct

This project adheres to a [Code of Conduct](CODE_OF_CONDUCT.md). By participating, you are expected to uphold this code.

## Contributing

Contributions are welcome! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines. Please note that all contributions require a Developer Certificate of Origin (DCO) sign-off. See [DCO](DCO) for more details.

## Acknowledgments

Cebu is inspired by research in computational topology, narrative theory, and spatial data structures.

## Version History

See [CHANGELOG.md](CHANGELOG.md) for the complete version history.

## Contact

For questions, issues, or suggestions, please open an issue on GitHub.

---

**Cebu**: Where topology meets narrative.
