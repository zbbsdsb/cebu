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
- `style`: Code formatting (no functionality change)
- `refactor`: Code refactoring
- `test`: Test-related
- `chore`: Build/tool-related
- `perf`: Performance optimization

### Example

```
feat(core): add spatial indexing with BVH tree

Implement BVH (Bounding Volume Hierarchy) tree for efficient spatial queries.
This provides 100-667x speedup for range queries compared to linear search.

- Add BVHTree class with SAH build strategy
- Implement nearest neighbor queries
- Add range query support
- Include comprehensive tests

Closes #123
```

## Developer Certificate of Origin (DCO)

All contributions must include a Developer Certificate of Origin (DCO) sign-off. This certifies that you have the right to submit the contribution under the project's license.

To sign your commits:

```bash
# Sign individual commits
git commit -s -m "Your commit message"

# Or configure Git to auto-sign all commits
git config --local user.name "Your Name"
git config --local user.email "your.email@example.com"
```

Your commit message must include:
```
Signed-off-by: Your Name <your.email@example.com>
```

For more information, see the [DCO](DCO) file.

## Pull Request Process

### Before Submitting

1. **Update Documentation** - Ensure all changes are documented
2. **Add Tests** - New features must include tests
3. **Run Tests Locally** - Verify all tests pass
4. **Check Code Style** - Run clang-format if needed
5. **Update CHANGELOG** - Add entry to CHANGELOG.md if applicable

### PR Requirements

- Clear title and description
- Reference related issues
- Include screenshots or test results if applicable
- All CI checks must pass
- At least one approval from a maintainer
- No merge conflicts

### PR Template

When creating a PR, please include:

```markdown
## Description
Brief description of changes

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Breaking change
- [ ] Documentation update

## Testing
- [ ] Tests added/updated
- [ ] All tests pass locally

## Checklist
- [ ] Code follows style guidelines
- [ ] Documentation updated
- [ ] CHANGELOG.md updated
- [ ] DCO signed
```

## Code Review

### Review Process

1. **Automated Checks** - CI runs tests and style checks
2. **Peer Review** - At least one maintainer reviews the code
3. **Feedback** - Address any review comments
4. **Approval** - Maintainer approves the PR
5. **Merge** - PR is merged into main branch

### Review Criteria

- Code quality and readability
- Test coverage and correctness
- Documentation completeness
- Performance implications
- Breaking changes
- Security considerations

## Documentation Contributions

### Types of Documentation

- **API Documentation** - Document public APIs
- **User Guide** - Help users get started
- **Examples** - Code examples and tutorials
- **Architecture** - System design documents

### Documentation Standards

- Use clear and concise language
- Include code examples where appropriate
- Keep documentation up-to-date with code changes
- Use proper Markdown formatting

### Building Documentation

```bash
# Build documentation locally
cd docs
mkdocs serve  # For MkDocs
# or
make html     # For Sphinx
```

## Reporting Issues

### Before Reporting

1. **Search Existing Issues** - Check if the issue has already been reported
2. **Gather Information** - Collect relevant details