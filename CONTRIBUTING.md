# Contributing to Cebu

We welcome contributions to the Cebu library! Here are some guidelines to help you get started.

## Getting Started

1. **Fork the repository** on GitHub.
2. **Clone your fork** to your local machine:
   ```bash
   git clone https://github.com/yourusername/cebu.git
   cd cebu
   ```
3. **Create a branch** for your feature or bug fix:
   ```bash
   git checkout -b feature/your-feature-name
   ```

## Developer Certificate of Origin (DCO)

All contributions must include a Developer Certificate of Origin (DCO) sign-off. This certifies that you have the right to submit the contribution under the project's license.

To sign your commits, you can:

1. **Sign individual commits**:
   ```bash
   git commit -s -m "Your commit message"
   ```

2. **Configure Git to auto-sign all commits**:
   ```bash
   git config --local commit.gpgsign false
   git config --local user.name "Your Name"
   git config --local user.email "your.email@example.com"
   ```

Your commit message must include:
```
Signed-off-by: Your Name <your.email@example.com>
```

For more information, see the [DCO](DCO) file.

## Coding Standards

- **Style**: Follow the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html).
- **Comments**: Add comments for complex business logic, but avoid unnecessary comments for obvious code.
- **Error Handling**: All asynchronous operations must have try-catch or explicit error return mechanisms.
- **Testing**: Each functional unit must have corresponding unit tests.

## Development Workflow

1. **Phase 1: Architecture First**:
   - Before writing any business logic, update `PROJECT_STRUCTURE.md` and core interface definitions if necessary.
   - Wait for user confirmation on architecture before proceeding.

2. **Phase 2: Modular Implementation**:
   - Focus on one file or function at a time.
   - Follow the "Read -> Plan -> Code" cycle: read related dependency files, outline the modification plan, then write code.

3. **Phase 3: Iterative Refinement**:
   - After generating code, self-review for type safety, error handling, and edge cases.
   - If multiple files are modified, list them step by step and clearly explain dependencies between files.

## Submitting a Pull Request

1. **Commit your changes** with clear commit messages:
   ```bash
   git add .
   git commit -m "Feature: Add X functionality"
   ```

2. **Push your branch** to GitHub:
   ```bash
   git push origin feature/your-feature-name
   ```

3. **Create a Pull Request** on GitHub:
   - Provide a clear title and description of your changes
   - Reference any related issues
   - Include screenshots or test results if applicable

## Testing

- Run the test suite to ensure your changes don't break existing functionality:
  ```bash
  mkdir build && cd build
  cmake ..
  cmake --build . --config Release
  ctest --output-on-failure
  ```

- Add new tests for any new functionality you implement.

## Documentation

- Update the documentation if you add new features or change existing functionality.
- Ensure all public APIs are documented.

## Reporting Issues

If you find a bug or have a feature request:

1. **Check existing issues** to see if it has already been reported.
2. **Create a new issue** with:
   - A clear title and description
   - Steps to reproduce (for bugs)
   - Expected behavior
   - Actual behavior
   - Environment details (compiler, OS, etc.)

## Code of Conduct

Please be respectful and constructive in all communications.

---

Thank you for contributing to Cebu!
