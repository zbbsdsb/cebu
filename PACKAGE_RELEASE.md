# Cebu Library Package Release Documentation

## Overview
Cebu is an advanced Simplicial Complex Library with spatial indexing, narrative-driven topology, and absurdity system. This document provides instructions for installing and using Cebu through various package managers.

## Package Managers

### 1. CMake

#### Installation
```bash
# Build and install Cebu
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/path/to/install
cmake --build . --config Release
cmake --install . --config Release

# For Debug build
cmake --build . --config Debug
cmake --install . --config Debug
```

#### Usage
```cmake
# In your CMakeLists.txt
find_package(cebu CONFIG REQUIRED)
add_executable(your_app main.cpp)
target_link_libraries(your_app cebu::cebu_core)
```

### 2. Conan

#### Installation
```bash
# From source
conan create . --name cebu --version 0.8.0

# From local cache
conan install cebu/0.8.0
```

#### Usage
```cmake
# In your CMakeLists.txt
find_package(cebu CONFIG REQUIRED)
add_executable(your_app main.cpp)
target_link_libraries(your_app cebu::cebu)
```

### 3. vcpkg

#### Installation
```bash
# Add the port file to your vcpkg ports directory
cp vcpkg/vcpkg.json /path/to/vcpkg/ports/cebu/

# Install Cebu
vcpkg install cebu

# With ZLIB support
vcpkg install cebu[zlib]
```

#### Usage
```cmake
# In your CMakeLists.txt
find_package(cebu CONFIG REQUIRED)
add_executable(your_app main.cpp)
target_link_libraries(your_app cebu::cebu_core)
```

## Dependencies

### Required
- **nlohmann_json** (3.2.0 or higher) - For JSON serialization

### Optional
- **ZLIB** - For compression support

## Version Information

- **Current Version**: 0.8.0
- **CMake Minimum Required**: 3.14
- **C++ Standard**: C++20

## Usage Examples

### Basic Usage
```cpp
#include <cebu/simplicial_complex.h>

int main() {
    cebu::SimplicialComplex sc;
    auto v1 = sc.add_vertex();
    auto v2 = sc.add_vertex();
    sc.add_edge(v1, v2);
    return 0;
}
```

### With Spatial Indexing
```cpp
#include <cebu/simplicial_complex.h>
#include <cebu/bvh.h>

int main() {
    cebu::SimplicialComplex sc;
    // Add vertices with positions
    auto v1 = sc.add_vertex({0.0, 0.0, 0.0});
    auto v2 = sc.add_vertex({1.0, 0.0, 0.0});
    auto v3 = sc.add_vertex({0.0, 1.0, 0.0});
    
    // Add triangle
    sc.add_triangle(v1, v2, v3);
    
    // Build BVH for spatial queries
    cebu::BVH bvh(sc);
    // Perform spatial queries...
    
    return 0;
}
```

## Build Options

| Option | Description | Default |
|--------|-------------|---------|
| CEBU_BUILD_TESTS | Build test suite | OFF |
| CEBU_BUILD_EXAMPLES | Build examples | OFF |
| CEBU_WITH_ZLIB | Enable ZLIB compression | ON |

## Troubleshooting

### CMake find_package Issues
- Ensure Cebu is installed in a directory that CMake can find
- Set `CMAKE_PREFIX_PATH` to the installation directory
- Verify that `cebuConfig.cmake` exists in the installation directory

### Conan Issues
- Ensure Conan 2.x is installed
- Check that the Conan recipe is correctly configured
- Verify dependencies are properly resolved

### vcpkg Issues
- Ensure vcpkg is up to date
- Check that the port file is correctly placed
- Verify dependencies are available in vcpkg

## Support

For issues and feature requests, please visit the [GitHub repository](https://github.com/ceaserzhao/cebu).
