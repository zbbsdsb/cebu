# Contributing to Cebu

Thank you for your interest in contributing to the Cebu project! This document will help you understand how to participate in project development.

## 📋 Table of Contents

- [Code of Conduct](#code-of-conduct)
- [How to Contribute](#how-to-contribute)
- [Development Environment Setup](#development-environment-setup)
- [Code Style Guide](#code-style-guide)
- [Testing Requirements](#testing-requirements)
- [Commit Guidelines](#commit-guidelines)
- [Pull Request Process](#pull-request-process)
- [Code Review](#code-review)
- [Documentation Contributions](#documentation-contributions)
- [Reporting Issues](#reporting-issues)

## Code of Conduct

This project adheres to the [Contributor Covenant](CODE_OF_CONDUCT.md) code of conduct. By participating in this project, you agree to abide by its terms.

## How to Contribute

### Types of Contributions

We welcome the following types of contributions:

- 🐛 **Bug Fixes** - Fix existing issues
- ✨ **New Features** - Add new capabilities
- 📚 **Documentation Improvements** - Enhance docs and examples
- 🎨 **Code Optimization** - Improve code quality and performance
- 🧪 **Test Enhancements** - Add test cases
- 🌐 **Translations** - Multi-language support

### Contribution Workflow

1. **Fork the Repository** - Fork the project on GitHub
2. **Clone Your Fork** - Clone your fork locally
   ```bash
   git clone https://github.com/yourusername/cebu.git
   cd cebu
   ```
3. **Create a Branch** - Create a new branch for your changes
   ```bash
   git checkout -b feature/your-feature-name
   # or
   git checkout -b fix/your-bug-fix
   ```
4. **Make Changes** - Write code and tests
5. **Commit Changes** - Use conventional commit messages
6. **Push Branch** - Push to your fork
7. **Create PR** - Create a Pull Request on GitHub

## Development Environment Setup

### System Requirements

- **Compiler**: GCC 10+, Clang 10+, or MSVC 2019+
- **CMake**: 3.14 or higher
- **Git**: For version control
- **Optional**: ZLIB (for compression support)

### Build Steps

```bash
# Clone repository (with submodules)
git clone --recursive https://github.com/yourusername/cebu.git
cd cebu

# Create build directory
mkdir build && cd build

# Configure project
cmake .. -DCEBU_BUILD_TESTS=ON -DCEBU_BUILD_EXAMPLES=ON

# Build
cmake --build . --config Release

# Run tests
ctest --output-on-failure

# Install (optional)
cmake --install .
```

### IDE Configuration

#### Visual Studio Code
Recommended extensions:
- C/C++ Extension Pack
- CMake Tools
- Clang-Format

#### Visual Studio
- Ensure C++ development workload is installed
- Use "Open Folder" feature to open project root

#### CLion
- Open project root directory directly
- CLion will automatically recognize CMake configuration

## Code Style Guide

### C++ Code Standards

Follow the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) with these key points:

#### Naming Conventions

```cpp
// Class names: PascalCase
class SimplicialComplex { };

// Function names: snake_case
void add_simplex();

// Variable names: snake_case
int simplex_count = 0;

// Constants: kPascalCase
const int kMaxSimplices = 1000;

// Member variables: snake_case_with_trailing_underscore
int vertex_count_;

// Enums: PascalCase
enum class BuildStrategy {
    Sah,
    Median,
    Linear
};
```

#### Formatting

- Use 4 spaces for indentation, no tabs
- Maximum 100 characters per line
- Opening braces on the same line
- Use clang-format for automatic formatting

```cpp
// Correct example
class Example {
public:
    void method() {
        if (condition) {
            do_something();
        }
    }
};
```

#### Comment Standards

```cpp
// Single-line comments: for brief explanations

/**
 * Multi-line comments: for detailed descriptions
 * Including parameters, return values, exceptions, etc.
 * 
 * @param id The simplex identifier
 * @return True if successful, false otherwise
 * @throws std::runtime_error If simplex not found
 */
bool remove_simplex(SimplexID id);
```

#### Error Handling

```cpp
// Use exceptions for error conditions
void risky_operation() {
    if (!is_valid()) {
        throw std::runtime_error("Invalid state for operation");
    }
    // Continue operation
}

// Use optional for operations that may fail
std::optional<SimplexID> find_simplex(const Criteria& criteria) {
    if (found) {
        return simplex_id;
    }
    return std::nullopt;
}
```

#### Smart Pointers

```cpp
// Prefer smart pointers
auto ptr = std::make_unique<Simplex>();
auto shared = std::make_shared<Complex>();

// Avoid raw pointers unless necessary
Simplex* raw_ptr = nullptr;  // Only for non-ownership scenarios
```

### Header File Standards

```cpp
#ifndef CEBU_SIMPLICIAL_COMPLEX_H_
#define CEBU_SIMPLICIAL_COMPLEX_H_

#include <vector>
#include <memory>

#include "cebu/types.h"
#include "cebu/simplex.h"

namespace cebu {

class SimplicialComplex {
public:
    // Public interface
private:
    // Private implementation
};

}  // namespace cebu

#endif  // CEBU_SIMPLICIAL_COMPLEX_H_
```

## Testing Requirements

### Test Types

1. **Unit Tests** - Test individual functions or classes
2. **Integration Tests** - Test module interactions
3. **Performance Tests** - Test performance benchmarks

### Test Naming

```cpp
// Test file naming: test_<module>.cpp
// Example: test_simplicial_complex.cpp

// Test function naming: test_<function>_<scenario>
TEST(SimplicialComplexTest, test_add_simplex_valid) {
    // Test code
}

TEST(SimplicialComplexTest, test_add_simplex_invalid) {
    // Test code
}
```

### Test Coverage

- New code should have at least 80% test coverage
- Critical paths must have test coverage
- Edge cases and exception scenarios must be tested

### Running Tests

```bash
# Run all tests
ctest --output-on-failure

# Run specific tests
ctest -R test_simplicial_complex

# Run with verbose output
ctest -V

# Generate coverage report (requires configuration)
cmake --build . --target coverage
```

### Test Example

```cpp
#include <gtest/gtest.h>
#include "cebu/simplicial_complex.h"

class SimplicialComplexTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup before test
    }
    
    void TearDown() override {
        // Cleanup after test
    }
    
    cebu::SimplicialComplex complex_;
};

TEST_F(SimplicialComplexTest, AddVertexIncreasesCount) {
    EXPECT_EQ(complex_.vertex_count(), 0);
    
    auto v = complex_.add_vertex();
    
    EXPECT_EQ(complex_.vertex_count(), 1);
    EXPECT_TRUE(complex_.has_simplex(v));
}

TEST_F(SimplicialComplexTest, RemoveNonExistentSimplexThrows) {
    EXPECT_THROW(complex_.remove_simplex(999), std::runtime_error);
}
```

## Commit Guidelines

### Commit Message Format

```
<type>(<scope>): <subject>

<body>

<footer>
```

### Type Categories

- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation update
