# Best Practices

This document provides best practices for using Cebu effectively in your projects.

## Table of Contents

1. [General Guidelines](#general-guidelines)
2. [Performance Optimization](#performance-optimization)
3. [Memory Management](#memory-management)
4. [Error Handling](#error-handling)
5. [Code Organization](#code-organization)
6. [Testing Strategies](#testing-strategies)
7. [Integration with Other Libraries](#integration-with-other-libraries)
8. [Advanced Usage Patterns](#advanced-usage-patterns)

---

## General Guidelines

### 1. Use C++20 Features

Cebu is designed to take advantage of C++20 features. Make sure to compile with C++20 support:

```cmake
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

### 2. Prefer Named Functions Over Direct Simplex Creation

Use the named functions for simplex creation instead of the generic `add_simplex` method:

```cpp
// Good
auto v0 = complex.add_vertex();
auto edge = complex.add_edge(v0, v1);
auto triangle = complex.add_triangle(v0, v1, v2);

// Avoid
auto v0 = complex.add_simplex({});
auto edge = complex.add_simplex({v0, v1});
auto triangle = complex.add_simplex({v0, v1, v2});
```

### 3. Use Labeled Complexes When Needed

Use `SimplicialComplexLabeled` when you need to attach data to simplices:

```cpp
// For complexes with labels
cebu::SimplicialComplexLabeled<float> complex;

// For complexes without labels
cebu::SimplicialComplex complex;
```

### 4. Handle Edge Cases

Always handle edge cases, especially when working with simplices:

```cpp
// Check if a simplex exists before using it
if (complex.has_simplex(simplex_id)) {
    // Use simplex
}

// Check if vertices exist before creating higher-dimensional simplices
if (complex.has_simplex(v0) && complex.has_simplex(v1) && complex.has_simplex(v2)) {
    auto triangle = complex.add_triangle(v0, v1, v2);
}
```

---

## Performance Optimization

### 1. Use Spatial Indexing for Large Complexes

For complexes with many simplices, use spatial indexing to accelerate queries:

```cpp
// For large complexes
cebu::SimplicialComplex sc;
cebu::VertexGeometry vg;
// Add many vertices and simplices...

// Build BVH for fast spatial queries
cebu::BVH bvh(sc, vg);

// Use for spatial queries
auto nearest = bvh.nearest_neighbor(query_point);
```

### 2. Batch Operations

Batch operations when possible to reduce overhead:

```cpp
// Good: Batch vertex creation
std::vector<cebu::SimplexID> vertices;
for (int i = 0; i < 1000; ++i) {
    vertices.push_back(complex.add_vertex());
}

// Good: Batch triangle creation
for (int i = 0; i < 998; ++i) {
    complex.add_triangle(vertices[i], vertices[i+1], vertices[i+2]);
}
```

### 3. Choose the Right Build Strategy

For BVH, choose the appropriate build strategy based on your use case:

```cpp
// For static scenes where query performance is critical
cebu::BVH bvh(sc, vg, cebu::BVH::BuildStrategy::SAH);

// For dynamic scenes where build speed is critical
cebu::BVH bvh(sc, vg, cebu::BVH::BuildStrategy::Median);
```

### 4. Use Parallel Building

Enable parallel building for large datasets:

```cpp
// Enable parallel BVH build
cebu::BVH bvh(sc, vg, cebu::BVH::BuildStrategy::SAH, true);

// Enable parallel Octree build
cebu::Octree octree(sc, vg, 10, true);
```

---

## Memory Management

### 1. Use Appropriate Data Structures

Choose the right data structures for your use case:

```cpp
// For small complexes
cebu::SimplicialComplex complex;

// For complexes with spatial queries
cebu::SimplicialComplex complex;
cebu::VertexGeometry vg;
cebu::BVH bvh(complex, vg);

// For complexes with labels
cebu::SimplicialComplexLabeled<float> complex;
```

### 2. Release Unused Resources

Release resources when they're no longer needed:

```cpp
// Clear geometry data when done
vg.clear();

// Rebuild indexes only when necessary
bvh.rebuild();
```

### 3. Avoid Unnecessary Copies

Use move semantics and references to avoid unnecessary copies:

```cpp
// Good: Use reference
const auto& vertices = complex.get_vertices();

// Good: Use move semantics
cebu::SimplicialComplex move_complex = std::move(complex);
```

---

## Error Handling

### 1. Check Return Values

Always check return values for functions that can fail:

```cpp
// Check if simplex creation succeeded
auto simplex = complex.add_simplex({v0, v1, v2});
if (simplex != cebu::INVALID_SIMPLEX) {
    // Simplex created successfully
}

// Check if serialization succeeded
bool success = cebu::serialization::save_to_json_file(complex, "complex.json");
if (success) {
    // Serialization succeeded
}
```

### 2. Use Exceptions Wisely

Use exceptions for exceptional cases, not for normal control flow:

```cpp
try {
    // Operation that might throw
    auto value = context.get_value<int>("key");
} catch (const std::out_of_range& e) {
    // Handle missing key
    std::cerr << "Key not found: " << e.what() << std::endl;
} catch (const std::bad_cast& e) {
    // Handle type mismatch
    std::cerr << "Type mismatch: " << e.what() << std::endl;
}
```

### 3. Validate Inputs

Validate inputs before passing them to Cebu functions:

```cpp
// Validate vertex IDs before creating an edge
if (complex.has_simplex(v0) && complex.has_simplex(v1)) {
    auto edge = complex.add_edge(v0, v1);
} else {
    // Handle invalid vertices
}
```

---

## Code Organization

### 1. Modular Design

Organize your code into modular components:

```cpp
// Geometry module
class GeometryManager {
public:
    void add_vertex(float x, float y, float z) {
        auto id = complex.add_vertex();
        vg.set_position(id, {x, y, z});
    }
    
    cebu::BVH& get_bvh() { return bvh; }
    
private:
    cebu::SimplicialComplex complex;
    cebu::VertexGeometry vg;
    cebu::BVH bvh;
};

// Narrative module
class NarrativeManager {
public:
    void advance_time(double delta) {
        timeline.advance(delta);
        timeline.update();
    }
    
private:
    cebu::Timeline timeline;
    cebu::NarrativeContext context;
};
```

### 2. Use Namespaces

Use namespaces to avoid name conflicts:

```cpp
namespace my_project {
    using namespace cebu;
    
    class Scene {
    private:
        SimplicialComplex complex;
        VertexGeometry vg;
    };
}
```

### 3. Document Your Code

Document your code, especially when using advanced Cebu features:

```cpp
/**
 * Builds a BVH for spatial queries
 * 
 * @param complex The simplicial complex to index
 * @param vg The vertex geometry with positions
 * @return BVH instance ready for queries
 */
cebu::BVH build_spatial_index(cebu::SimplicialComplex& complex, cebu::VertexGeometry& vg) {
    return cebu::BVH(complex, vg, cebu::BVH::BuildStrategy::SAH, true);
}
```

---

## Testing Strategies

### 1. Unit Tests

Write unit tests for your Cebu-based code:

```cpp
TEST(SimplicialComplexTest, AddVertex) {
    cebu::SimplicialComplex complex;
    auto v0 = complex.add_vertex();
    EXPECT_NE(v0, cebu::INVALID_SIMPLEX);
    EXPECT_EQ(complex.vertex_count(), 1);
}

TEST(SimplicialComplexTest, AddTriangle) {
    cebu::SimplicialComplex complex;
    auto v0 = complex.add_vertex();
    auto v1 = complex.add_vertex();
    auto v2 = complex.add_vertex();
    auto tri = complex.add_triangle(v0, v1, v2);
    EXPECT_NE(tri, cebu::INVALID_SIMPLEX);
    EXPECT_EQ(complex.simplex_count(2), 1);
}
```

### 2. Integration Tests

Test integration with other components:

```cpp
TEST(SpatialIndexTest, NearestNeighbor) {
    cebu::SimplicialComplex complex;
    cebu::VertexGeometry vg;
    
    // Add vertices with positions
    auto v0 = complex.add_vertex();
    auto v1 = complex.add_vertex();
    vg.set_position(v0, {0.0, 0.0, 0.0});
    vg.set_position(v1, {1.0, 1.0, 1.0});
    
    // Build BVH
    cebu::BVH bvh(complex, vg);
    
    // Test nearest neighbor
    cebu::Point query = {0.1, 0.1, 0.1};
    auto nearest = bvh.nearest_neighbor(query);
    EXPECT_EQ(nearest, v0);
}
```

### 3. Performance Tests

Test performance for critical operations:

```cpp
TEST(PerformanceTest, BVHBuild) {
    cebu::SimplicialComplex complex;
    cebu::VertexGeometry vg;
    
    // Add many vertices
    for (int i = 0; i < 10000; ++i) {
        auto v = complex.add_vertex();
        vg.set_position(v, {static_cast<float>(i), 0.0, 0.0});
    }
    
    // Measure build time
    auto start = std::chrono::high_resolution_clock::now();
    cebu::BVH bvh(complex, vg);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    EXPECT_LT(duration, 1000); // Should build in less than 1 second
}
```

---

## Integration with Other Libraries

### 1. OpenGL/Vulkan Integration

Integrate with graphics libraries for visualization:

```cpp
// OpenGL integration
void render_complex(const cebu::SimplicialComplex& complex, const cebu::VertexGeometry& vg) {
    // Generate OpenGL buffers
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    // Fill buffers with data from complex
    for (auto v : complex.get_vertices()) {
        auto pos = vg.get_position(v);
        vertices.push_back(pos.x);
        vertices.push_back(pos.y);
        vertices.push_back(pos.z);
    }
    
    // Draw using OpenGL
    // ...
}
```

### 2. Physics Engine Integration

Integrate with physics engines:

```cpp
// Physics engine integration
void setup_physics(const cebu::SimplicialComplex& complex, const cebu::VertexGeometry& vg, PhysicsWorld& world) {
    // Create physics bodies for each simplex
    for (auto tri : complex.get_simplices_of_dimension(2)) {
        // Get triangle vertices
        auto vertices = complex.get_simplex_vertices(tri);
        if (vertices.size() == 3) {
            auto p0 = vg.get_position(vertices[0]);
            auto p1 = vg.get_position(vertices[1]);
            auto p2 = vg.get_position(vertices[2]);
            
            // Create physics triangle
            world.add_triangle(p0, p1, p2);
        }
    }
}
```

### 3. Serialization Integration

Integrate with custom serialization formats:

```cpp
// Custom serialization
void save_to_custom_format(const cebu::SimplicialComplex& complex, const std::string& filename) {
    std::ofstream file(filename);
    
    // Write vertices
    file << "Vertices: " << complex.vertex_count() << std::endl;
    for (auto v : complex.get_vertices()) {
        file << v << std::endl;
    }
    
    // Write triangles
    file << "Triangles: " << complex.simplex_count(2) << std::endl;
    for (auto tri : complex.get_simplices_of_dimension(2)) {
        auto vertices = complex.get_simplex_vertices(tri);
        file << tri << " " << vertices[0] << " " << vertices[1] << " " << vertices[2] << std::endl;
    }
}
```

---

## Advanced Usage Patterns

### 1. Command Pattern for Undo/Redo

Use the command pattern for undo/redo functionality:

```cpp
class AddSimplexCommand {
public:
    AddSimplexCommand(cebu::SimplicialComplex& complex, const std::vector<cebu::SimplexID>& faces)
        : complex_(complex), faces_(faces), simplex_id_(cebu::INVALID_SIMPLEX) {}
    
    void execute() {
        simplex_id_ = complex_.add_simplex(faces_);
    }
    
    void undo() {
        if (simplex_id_ != cebu::INVALID_SIMPLEX) {
            complex_.remove_simplex(simplex_id_);
        }
    }
    
private:
    cebu::SimplicialComplex& complex_;
    std::vector<cebu::SimplexID> faces_;
    cebu::SimplexID simplex_id_;
};
```

### 2. Event-Driven Architecture

Use the event system for reactive programming:

```cpp
class GameWorld {
public:
    GameWorld() {
        // Subscribe to complex events
        complex_.on_simplex_added.connect([this](cebu::SimplexID id) {
            std::cout << "Simplex added: " << id << std::endl;
            // Handle simplex added event
        });
        
        complex_.on_simplex_removed.connect([this](cebu::SimplexID id) {
            std::cout << "Simplex removed: " << id << std::endl;
            // Handle simplex removed event
        });
    }
    
private:
    cebu::SimplicialComplex complex_;
};
```

### 3. Narrative-Driven Topology

Use narrative features for dynamic topology:

```cpp
class GameLevel {
public:
    GameLevel() {
        // Create timeline with events
        timeline_.add_event(create_spawn_event(0.0));
        timeline_.add_event(create_boss_event(60.0));
        timeline_.add_event(create_victory_event(120.0));
    }
    
    void update(double delta) {
        timeline_.advance(delta);
        timeline_.update();
    }
    
private:
    cebu::SimplicialComplex complex_;
    cebu::Timeline timeline_;
    cebu::NarrativeContext context_;
    
    cebu::StoryEvent create_spawn_event(double time) {
        cebu::StoryEvent event(time, "Spawn Enemies");
        event.on_trigger.connect([this](double t) {
            // Spawn enemies by adding new simplices
        });
        return event;
    }
    
    // Other event creation methods...
};
```

### 4. Custom Geometry Providers

Create custom geometry providers for specialized data:

```cpp
class TerrainGeometry : public cebu::GeometryProvider {
public:
    TerrainGeometry(const Heightmap& heightmap) : heightmap_(heightmap) {}
    
    cebu::Point get_position(cebu::SimplexID id) const override {
        // Convert simplex ID to heightmap coordinates
        int x = id % heightmap_.width;
        int y = id / heightmap_.width;
        float z = heightmap_.get_height(x, y);
        return {static_cast<float>(x), static_cast<float>(y), z};
    }
    
private:
    const Heightmap& heightmap_;
};

// Usage
Heightmap heightmap(100, 100);
TerrainGeometry terrain_geometry(heightmap);
cebu::BVH bvh(complex, terrain_geometry);
```

---

## Conclusion

By following these best practices, you can make the most of Cebu's features and create efficient, maintainable code. Remember to adapt these guidelines to your specific use case and always measure performance to ensure your implementation meets your requirements.

For more information, see the [API Documentation](../api/api_core.md) and [Performance Guide](performance.md).

---

**Cebu Version**: 0.8.0
**Last Updated**: 2026-03-18