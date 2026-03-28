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
void add_simple