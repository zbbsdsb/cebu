# Installation Guide

This guide provides detailed instructions for installing Cebu on various platforms.

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Building from Source](#building-from-source)
3. [Using Package Managers](#using-package-managers)
4. [Verifying the Installation](#verifying-the-installation)
5. [Troubleshooting](#troubleshooting)

---

## Prerequisites

Before installing Cebu, ensure you have the following dependencies:

### Core Dependencies
- **C++20 compatible compiler**
  - GCC 10+ (Linux/macOS)
  - Clang 10+ (Linux/macOS)
  - MSVC 2019+ (Windows)
- **CMake 3.18 or higher**
- **Git** (optional, for cloning the repository)

### Optional Dependencies
- **nlohmann_json** (for JSON serialization)
  - Version 3.10.0 or higher
- **ZLIB** (for compressed serialization)
  - Version 1.2.11 or higher

---

## Building from Source

### Linux / macOS

1. **Clone the repository**
   ```bash
   git clone https://github.com/yourusername/cebu.git
   cd cebu
   ```

2. **Create build directory**
   ```bash
   mkdir build && cd build
   ```

3. **Configure with CMake**
   ```bash
   cmake .. -DCMAKE_BUILD_TYPE=Release
   ```

4. **Build the library**
   ```bash
   cmake --build . -j$(nproc)
   ```

5. **Run tests (optional)**
   ```bash
   ctest --output-on-failure
   ```

6. **Install the library**
   ```bash
   sudo cmake --install .
   ```

### Windows (Visual Studio)

1. **Clone the repository**
   ```batch
   git clone https://github.com/yourusername/cebu.git
   cd cebu
   ```

2. **Create build directory**
   ```batch
   mkdir build
   cd build
   ```

3. **Configure with CMake**
   ```batch
   cmake .. -G "Visual Studio 16 2019" -A x64
   ```

4. **Build the library**
   ```batch
   cmake --build . --config Release
   ```

5. **Run tests (optional)**
   ```batch
   ctest -C Release --output-on-failure
   ```

6. **Install the library**
   ```batch
   cmake --install . --config Release
   ```

### CMake Options

| Option | Description | Default |
|--------|-------------|---------|
| `CEBU_BUILD_TESTS` | Build test suite | OFF |
| `CEBU_BUILD_EXAMPLES` | Build examples | OFF |
| `CEBU_ENABLE_SERIALIZATION` | Enable serialization support | ON |
| `CEBU_ENABLE_SPATIAL_INDEXING` | Enable spatial indexing features | ON |
| `CEBU_ENABLE_NARRATIVE` | Enable narrative features | ON |
| `CEBU_ENABLE_ABSURDITY` | Enable absurdity system | ON |
| `CEBU_ENABLE_VERSION_CONTROL` | Enable version control features | ON |

Example usage:
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release -DCEBU_BUILD_TESTS=ON -DCEBU_BUILD_EXAMPLES=ON
```

---

## Using Package Managers

### vcpkg

1. **Install vcpkg** (if not already installed)
   ```bash
   git clone https://github.com/microsoft/vcpkg.git
   cd vcpkg
   ./bootstrap-vcpkg.sh  # Linux/macOS
   bootstrap-vcpkg.bat    # Windows
   ```

2. **Install Cebu**
   ```bash
   # From the vcpkg directory
   ./vcpkg install cebu
   ```

3. **Use in CMake project**
   ```cmake
   cmake_minimum_required(VERSION 3.18)
   project(MyProject)
   
   set(CMAKE_CXX_STANDARD 20)
   
   # Specify vcpkg toolchain
   set(CMAKE_TOOLCHAIN_FILE /path/to/vcpkg/scripts/buildsystems/vcpkg.cmake)
   
   # Find Cebu
   find_package(cebu REQUIRED)
   
   # Link to Cebu
   add_executable(my_app main.cpp)
   target_link_libraries(my_app cebu::cebu)
   ```

### Conan

1. **Install Conan** (if not already installed)
   ```bash
   pip install conan
   ```

2. **Add the Conan repository** (if not already configured)
   ```bash
   conan remote add myrepo https://your-conan-repo.com
   ```

3. **Install Cebu**
   ```bash
   conan install cebu/0.8.0@yourusername/stable --build=missing
   ```

4. **Use in CMake project**
   ```cmake
   cmake_minimum_required(VERSION 3.18)
   project(MyProject)
   
   set(CMAKE_CXX_STANDARD 20)
   
   # Include Conan generated file
   include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
   conan_basic_setup()
   
   # Link to Cebu
   add_executable(my_app main.cpp)
   target_link_libraries(my_app ${CONAN_LIBS})
   ```

---

## Verifying the Installation

To verify that Cebu is installed correctly, create a simple test program:

### Test Program

```cpp
#include "cebu/simplicial_complex.h"
#include <iostream>

int main() {
    cebu::SimplicialComplex complex;
    
    // Add a few vertices
    auto v0 = complex.add_vertex();
    auto v1 = complex.add_vertex();
    auto v2 = complex.add_vertex();
    
    // Add a triangle
    auto tri = complex.add_triangle(v0, v1, v2);
    
    std::cout << "Cebu installation verified!" << std::endl;
    std::cout << "Created complex with " << complex.vertex_count() << " vertices" << std::endl;
    std::cout << "Added triangle with ID: " << tri << std::endl;
    
    return 0;
}
```

### Compile and Run

#### Linux / macOS
```bash
g++ -std=c++20 -I /usr/local/include -L /usr/local/lib -o test_cebu test_cebu.cpp -lcebu_core
./test_cebu
```

#### Windows
```batch
cl /std:c++20 /I "C:\Program Files\cebu\include" test_cebu.cpp /link /LIBPATH:"C:\Program Files\cebu\lib" cebu_core.lib
.	est_cebu.exe
```

### Expected Output
```
Cebu installation verified!
Created complex with 3 vertices
Added triangle with ID: 3
```

---

## Troubleshooting

### Common Issues

1. **Compiler Compatibility**
   - **Issue**: Compiler does not support C++20
   - **Solution**: Update your compiler to a C++20 compatible version

2. **CMake Version**
   - **Issue**: CMake version too old
   - **Solution**: Install CMake 3.18 or higher

3. **Missing Dependencies**
   - **Issue**: nlohmann_json or ZLIB not found
   - **Solution**: Install the required dependencies or disable the corresponding features

4. **Linker Errors**
   - **Issue**: Cannot find cebu_core library
   - **Solution**: Ensure the library is installed in a standard location or specify the path

5. **Package Manager Issues**
   - **Issue**: vcpkg/conan cannot find Cebu
   - **Solution**: Check that you're using the correct repository and version

### Debugging Tips

- **Verbose CMake Output**
  ```bash
  cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_VERBOSE_MAKEFILE=ON
  ```

- **Check CMake Find Package**
  ```bash
  cmake --find-package -DNAME=cebu -DCOMPILER_ID=GNU -DLANGUAGE=CXX -DMODE=EXIST
  ```

- **Inspect Installation**
  ```bash
  # Linux/macOS
  find /usr/local -name "cebu*"
  
  # Windows
  dir "C:\Program Files\cebu"
  ```

---

## Advanced Installation

### Custom Installation Path

```bash
cmake .. -DCMAKE_INSTALL_PREFIX=/path/to/custom/location
cmake --build .
cmake --install .
```

### Static vs Shared Library

By default, Cebu builds as a shared library. To build as a static library:

```bash
cmake .. -DBUILD_SHARED_LIBS=OFF
```

### Cross-Compilation

```bash
cmake .. -DCMAKE_TOOLCHAIN_FILE=/path/to/toolchain.cmake
```

---

## Contributing to Cebu

If you encounter any issues during installation, please report them on the [GitHub Issues](https://github.com/yourusername/cebu/issues) page.

For more information on contributing to Cebu, see the [Contributing Guide](utilities/contributing.md).

---

**Cebu Version**: 0.8.0
**Last Updated**: 2026-03-18