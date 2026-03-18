# Troubleshooting Guide

This document provides solutions to common issues you might encounter when using Cebu.

## Table of Contents

1. [Installation Issues](#installation-issues)
2. [Build Errors](#build-errors)
3. [Runtime Errors](#runtime-errors)
4. [Performance Issues](#performance-issues)
5. [Serialization Issues](#serialization-issues)
6. [Spatial Indexing Issues](#spatial-indexing-issues)
7. [Narrative Features Issues](#narrative-features-issues)
8. [General Troubleshooting](#general-troubleshooting)

---

## Installation Issues

### 1. CMake Cannot Find Cebu

**Issue**: CMake fails to find Cebu with `find_package(cebu)`

**Possible Causes**:
- Cebu is not installed
- Cebu is installed in a non-standard location
- CMake cannot find the Cebu package configuration file

**Solutions**:
- Ensure Cebu is properly installed
- Specify the Cebu installation directory:
  ```cmake
  set(cebu_DIR /path/to/cebu/lib/cmake/cebu)
  find_package(cebu REQUIRED)
  ```
- Use the `CMAKE_PREFIX_PATH` variable:
  ```bash
  cmake .. -DCMAKE_PREFIX_PATH=/path/to/cebu
  ```

### 2. Missing Dependencies

**Issue**: Build fails due to missing dependencies (nlohmann_json, ZLIB)

**Possible Causes**:
- Dependencies are not installed
- CMake cannot find the dependencies

**Solutions**:
- Install the required dependencies:
  - **nlohmann_json**: `vcpkg install nlohmann-json` or download from GitHub
  - **ZLIB**: `vcpkg install zlib` or install via system package manager
- Specify dependency locations:
  ```cmake
  set(nlohmann_json_DIR /path/to/nlohmann_json/lib/cmake/nlohmann_json)
  set(ZLIB_ROOT /path/to/zlib)
  ```

### 3. Package Manager Issues

**Issue**: Cannot install Cebu via vcpkg or Conan

**Possible Causes**:
- Package not available in the repository
- Network issues
- Incorrect package name or version

**Solutions**:
- For vcpkg: Ensure the port file is in the correct location
- For Conan: Ensure the recipe is properly configured
- Check network connectivity
- Verify the package name and version

---

## Build Errors

### 1. C++20 Not Supported

**Issue**: Compiler error about C++20 features

**Possible Causes**:
- Compiler does not support C++20
- CMake is not configured to use C++20

**Solutions**:
- Update your compiler to a C++20 compatible version:
  - GCC 10+
  - Clang 10+
  - MSVC 2019+
- Ensure CMake is configured for C++20:
  ```cmake
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  ```

### 2. Undefined Symbols

**Issue**: Linker errors about undefined symbols

**Possible Causes**:
- Missing library link
- Incorrect library name
- Mismatched library versions

**Solutions**:
- Ensure you're linking to the correct library:
  ```cmake
  target_link_libraries(your_target cebu::cebu)
  ```
- Check that the library was built correctly
- Verify library versions match

### 3. Header Not Found

**Issue**: Compiler cannot find Cebu headers

**Possible Causes**:
- Include directories not set
- Headers not installed
- Incorrect include path

**Solutions**:
- Add Cebu include directory:
  ```cmake
  target_include_directories(your_target PRIVATE /path/to/cebu/include)
  ```
- Ensure Cebu is properly installed
- Check include statements in your code:
  ```cpp
  #include "cebu/simplicial_complex.h"  // Correct
  #include "simplicial_complex.h"        // Incorrect
  ```

---

## Runtime Errors

### 1. Invalid Simplex ID

**Issue**: Error about invalid simplex ID

**Possible Causes**:
- Using a simplex ID that doesn't exist
- Using a simplex ID after it has been removed
- Invalid simplex creation parameters

**Solutions**:
- Check if the simplex exists before using it:
  ```cpp
  if (complex.has_simplex(simplex_id)) {
      // Use simplex
  }
  ```
- Ensure all vertices exist before creating higher-dimensional simplices
- Check return values from simplex creation functions:
  ```cpp
  auto simplex = complex.add_simplex(faces);
  if (simplex == cebu::INVALID_SIMPLEX) {
      // Handle error
  }
  ```

### 2. Memory Access Violation

**Issue**: Segmentation fault or access violation

**Possible Causes**:
- Using invalid simplex IDs
- Accessing deleted simplices
- Invalid geometry data
- Stack overflow from deep recursion

**Solutions**:
- Validate all simplex IDs before use
- Ensure geometry data is properly initialized
- Check for infinite recursion in custom callbacks
- Use bounds checking for array access

### 3. Unexpected Behavior

**Issue**: Complex behavior doesn't match expectations

**Possible Causes**:
- Incorrect understanding of Cebu's API
- Wrong parameters passed to functions
- Missing required initialization
- Race conditions in multi-threaded code

**Solutions**:
- Review the API documentation
- Check function parameters and return values
- Ensure proper initialization of all components
- Use proper synchronization in multi-threaded code

---

## Performance Issues

### 1. Slow Build Times

**Issue**: BVH or Octree build is too slow

**Possible Causes**:
- Large number of simplices
- Complex build strategy
- Single-threaded build

**Solutions**:
- Use a faster build strategy for BVH:
  ```cpp
  cebu::BVH bvh(sc, vg, cebu::BVH::BuildStrategy::Median);
  ```
- Enable parallel building:
  ```cpp
  cebu::BVH bvh(sc, vg, cebu::BVH::BuildStrategy::SAH, true);
  ```
- Consider using Octree for uniformly distributed data

### 2. Slow Query Performance

**Issue**: Spatial queries are too slow

**Possible Causes**:
- Inefficient index structure
- Inappropriate index type
- Complex query parameters

**Solutions**:
- Use BVH for irregularly distributed data
- Use Octree for uniformly distributed data
- Choose the appropriate build strategy for BVH
- Batch queries when possible

### 3. High Memory Usage

**Issue**: Cebu uses too much memory

**Possible Causes**:
- Large number of simplices
- Detailed spatial indexes
- Memory leaks

**Solutions**:
- Release unused resources:
  ```cpp
  vg.clear();
  ```
- Use appropriate index types for your data
- Check for memory leaks in your code
- Consider using smaller data types where possible

---

## Serialization Issues

### 1. Failed to Save/Load

**Issue**: Serialization or deserialization fails

**Possible Causes**:
- Invalid file path
- Insufficient permissions
- Corrupted file
- Version mismatch

**Solutions**:
- Check file paths and permissions
- Verify the file is not corrupted
- Ensure the same Cebu version is used for save and load
- Handle serialization errors:
  ```cpp
  bool success = cebu::serialization::save_to_json_file(complex, "file.json");
  if (!success) {
      // Handle error
  }
  ```

### 2. Large File Sizes

**Issue**: Serialized files are too large

**Possible Causes**:
- Using JSON format for large complexes
- No compression enabled
- Redundant data

**Solutions**:
- Use binary format for large complexes:
  ```cpp
  cebu::serialization::save_to_binary_file(complex, "file.bin");
  ```
- Enable compression:
  ```cpp
  cebu::Persistence::save_to_file(complex, "file.cjson", true);
  ```
- Remove unnecessary data before serialization

### 3. Data Loss

**Issue**: Data is lost during serialization/deserialization

**Possible Causes**:
- Incomplete serialization implementation
- Version incompatibility
- Custom data not properly serialized

**Solutions**:
- Ensure all custom data is properly serialized
- Use version control for serialized files
- Test serialization/deserialization with your specific data

---

## Spatial Indexing Issues

### 1. Incorrect Query Results

**Issue**: Spatial queries return wrong results

**Possible Causes**:
- Incorrect geometry data
- Outdated index
- Incorrect query parameters

**Solutions**:
- Verify geometry data is correct:
  ```cpp
  auto pos = vg.get_position(vertex_id);
  ```
- Rebuild the index after modifying the complex:
  ```cpp
  bvh.rebuild();
  ```
- Check query parameters and coordinate systems

### 2. Index Rebuilding Too Slow

**Issue**: Rebuilding spatial indexes takes too long

**Possible Causes**:
- Large number of simplices
- Frequent modifications
- Complex build strategy

**Solutions**:
- Use incremental updates instead of full rebuilds:
  ```cpp
  bvh.insert(new_simplex);
  bvh.remove(deleted_simplex);
  ```
- Use a faster build strategy
- Batch modifications before rebuilding

### 3. Index Not Updating

**Issue**: Index doesn't reflect changes to the complex

**Possible Causes**:
- Forgetting to rebuild the index
- Incremental updates not working
- Index out of sync

**Solutions**:
- Rebuild the index after modifications:
  ```cpp
  bvh.rebuild();
  ```
- Use incremental updates for small changes
- Verify the index is using the correct complex and geometry

---

## Narrative Features Issues

### 1. Events Not Triggering

**Issue**: Story events don't trigger at the expected time

**Possible Causes**:
- Timeline not updated
- Event conditions not met
- Incorrect event time

**Solutions**:
- Ensure the timeline is updated regularly:
  ```cpp
  timeline.advance(delta);
  timeline.update();
  ```
- Check event conditions:
  ```cpp
  if (event.check_condition()) {
      // Condition is met
  }
  ```
- Verify event times are set correctly

### 2. Context Values Not Persisting

**Issue**: Narrative context values are lost

**Possible Causes**:
- Context not properly maintained
- Values not set correctly
- Scope issues

**Solutions**:
- Ensure context is properly scoped
- Check value types when setting and getting:
  ```cpp
  context.set_value("key", 42);  // int
  int value = context.get_value<int>("key");  // Correct type
  ```
- Verify values are set before accessing

### 3. Timeline Issues

**Issue**: Timeline not advancing or behaving incorrectly

**Possible Causes**:
- Incorrect time values
- Timeline not updated
- Milestones not properly set

**Solutions**:
- Ensure timeline is updated:
  ```cpp
  timeline.advance(delta);
  ```
- Check time values are within valid range
- Verify milestone times are correctly ordered

---

## General Troubleshooting

### 1. Enable Debug Mode

**Solution**:
Enable debug mode for more detailed error messages:

```cmake
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

### 2. Check Cebu Version

**Solution**:
Verify you're using the correct Cebu version:

```cpp
#include "cebu/version.h"
std::cout << "Cebu version: " << CEBU_VERSION << std::endl;
```

### 3. Run Tests

**Solution**:
Run the Cebu test suite to ensure basic functionality:

```bash
cd build
ctest --output-on-failure
```

### 4. Check Documentation

**Solution**:
Refer to the documentation for usage examples and API details:
- [API Documentation](../api/api_core.md)
- [Getting Started Guide](../getting_started.md)
- [Best Practices](best_practices.md)

### 5. Report Issues

**Solution**:
If you encounter a bug that you can't resolve, report it on the GitHub Issues page with:
- Cebu version
- Compiler and OS information
- Minimal reproducible example
- Expected vs actual behavior

---

## Common Error Messages

### 1. "Simplex not found"

**Cause**: Attempting to use a simplex ID that doesn't exist
**Solution**: Check if the simplex exists before using it

### 2. "Invalid simplex faces"

**Cause**: Attempting to create a simplex with invalid faces
**Solution**: Ensure all face simplex IDs exist and form a valid simplex

### 3. "Geometry not initialized"

**Cause**: Using spatial indexing without setting geometry data
**Solution**: Set positions for all vertices before building spatial indexes

### 4. "Serialization failed"

**Cause**: Error saving or loading complex
**Solution**: Check file permissions and path

### 5. "Timeline not updated"

**Cause**: Story events not triggering
**Solution**: Call `timeline.update()` after advancing time

---

## Conclusion

By following this troubleshooting guide, you should be able to resolve most issues you encounter when using Cebu. If you're still having problems, don't hesitate to reach out to the community for help.

For more information, see the [API Documentation](../api/api_core.md) and [Best Practices](best_practices.md).

---

**Cebu Version**: 0.8.0
**Last Updated**: 2026-03-18