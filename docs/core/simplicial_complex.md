# Simplicial Complex Basics

## Overview
A simplicial complex is a collection of simplices that satisfy the property that every face of a simplex in the complex is also in the complex. In Cebu, simplicial complexes are the fundamental data structure for representing topological spaces.

## Basic Concepts

### Simplices
- **0-simplex**: A vertex
- **1-simplex**: An edge
- **2-simplex**: A triangle
- **3-simplex**: A tetrahedron
- **n-simplex**: An n-dimensional simplex

### Simplex Representation
In Cebu, simplices are represented by their vertex sets. Each simplex is uniquely identified by its vertices, which are stored in a sorted order.

## Creating a Simplicial Complex

### Basic Construction
```cpp
#include "cebu/simplicial_complex.h"

// Create a simplicial complex
cebu::SimplicialComplex sc;

// Add vertices (0-simplices)
auto v1 = sc.add_vertex();
auto v2 = sc.add_vertex();
auto v3 = sc.add_vertex();

// Add an edge (1-simplex)
auto e1 = sc.add_edge(v1, v2);

// Add a triangle (2-simplex)
auto t1 = sc.add_triangle(v1, v2, v3);
```

### Adding Simplices

#### Adding Vertices
```cpp
// Add a vertex with default properties
auto v = sc.add_vertex();

// Add a vertex with a position (requires vertex geometry)
#include "cebu/vertex_geometry.h"
cebu::VertexGeometry vg;
auto v = sc.add_vertex();
vg.set_position(v, {1.0, 2.0, 3.0});
```

#### Adding Edges
```cpp
// Add an edge between two vertices
auto e = sc.add_edge(v1, v2);

// Add an edge with a label (using labeled complex)
#include "cebu/simplicial_complex_labeled.h"
cebu::SimplicialComplexLabeled<int> sc;
auto v1 = sc.add_vertex(0);
auto v2 = sc.add_vertex(1);
auto e = sc.add_edge(v1, v2, 10);
```

#### Adding Triangles
```cpp
// Add a triangle
auto t = sc.add_triangle(v1, v2, v3);
```

#### Adding Higher-Dimensional Simplices
```cpp
// Add a tetrahedron (3-simplex)
auto tet = sc.add_simplex({v1, v2, v3, v4});

// Add an n-simplex
auto simplex = sc.add_simplex({v1, v2, v3, v4, v5});
```

## Querying Simplices

### By Dimension
```cpp
// Get all vertices (0-simplices)
auto vertices = sc.get_simplices(0);

// Get all edges (1-simplices)
auto edges = sc.get_simplices(1);

// Get all triangles (2-simplices)
auto triangles = sc.get_simplices(2);
```

### By Adjacency
```cpp
// Get all neighbors of a vertex
auto neighbors = sc.get_neighbors(v1);

// Get all edges incident to a vertex
auto incident_edges = sc.get_incident_edges(v1);

// Get all triangles incident to an edge
auto incident_triangles = sc.get_incident_triangles(e1);
```

### By ID
```cpp
// Get a simplex by ID
auto simplex = sc.get_simplex(simplex_id);
```

## Removing Simplices

### Removing a Vertex
```cpp
// Remove a vertex (will also remove all incident simplices)
sc.remove_vertex(v1);
```

### Removing an Edge
```cpp
// Remove an edge (will also remove all incident triangles)
sc.remove_edge(e1);
```

### Removing a Triangle
```cpp
// Remove a triangle
sc.remove_triangle(t1);
```

### Removing a Simplex
```cpp
// Remove any simplex
sc.remove_simplex(simplex_id);
```

## Topological Properties

### Euler Characteristic
```cpp
// Compute Euler characteristic
int chi = sc.euler_characteristic();
```

### Manifold Checking
```cpp
// Check if the complex is a manifold
bool is_manifold = sc.is_manifold();

// Check if a specific simplex is on the boundary
bool is_boundary = sc.is_boundary(simplex_id);
```

### Connected Components
```cpp
// Get connected components
auto components = sc.connected_components();
```

## Performance Considerations

### Time Complexity
- **Adding a simplex**: O(k log n), where k is the dimension
- **Removing a simplex**: O(k log n)
- **Querying by dimension**: O(1)
- **Querying by adjacency**: O(k)

### Space Complexity
- O(n + m), where n is the number of vertices and m is the number of simplices

## Best Practices

1. **Batch Operations**: For large complexes, use batch operations to improve performance
2. **Spatial Indexing**: For spatial queries, use BVH or Octree
3. **Labeling**: Use labeled complexes for additional properties
4. **Serialization**: Save complex state using JSON or binary format

## Examples

### Basic Usage
```cpp
#include "cebu/simplicial_complex.h"

int main() {
    // Create a simplicial complex
    cebu::SimplicialComplex sc;
    
    // Add vertices
    auto v1 = sc.add_vertex();
    auto v2 = sc.add_vertex();
    auto v3 = sc.add_vertex();
    auto v4 = sc.add_vertex();
    
    // Add edges
    sc.add_edge(v1, v2);
    sc.add_edge(v2, v3);
    sc.add_edge(v3, v4);
    sc.add_edge(v4, v1);
    
    // Add a diagonal
    sc.add_edge(v1, v3);
    
    // Add triangles
    sc.add_triangle(v1, v2, v3);
    sc.add_triangle(v1, v3, v4);
    
    // Print information
    std::cout << "Vertices: " << sc.get_simplices(0).size() << std::endl;
    std::cout << "Edges: " << sc.get_simplices(1).size() << std::endl;
    std::cout << "Triangles: " << sc.get_simplices(2).size() << std::endl;
    std::cout << "Euler characteristic: " << sc.euler_characteristic() << std::endl;
    
    return 0;
}
```

### With Vertex Geometry
```cpp
#include "cebu/simplicial_complex.h"
#include "cebu/vertex_geometry.h"

int main() {
    cebu::SimplicialComplex sc;
    cebu::VertexGeometry vg;
    
    // Add vertices with positions
    auto v1 = sc.add_vertex();
    auto v2 = sc.add_vertex();
    auto v3 = sc.add_vertex();
    
    vg.set_position(v1, {0.0, 0.0, 0.0});
    vg.set_position(v2, {1.0, 0.0, 0.0});
    vg.set_position(v3, {0.0, 1.0, 0.0});
    
    // Add a triangle
    sc.add_triangle(v1, v2, v3);
    
    // Compute centroid
    auto centroid = vg.centroid(sc.get_simplices(2)[0]);
    std::cout << "Centroid: (" << centroid.x << ", " << centroid.y << ", " << centroid.z << ")" << std::endl;
    
    return 0;
}
```

## Conclusion

Simplicial complexes are the foundation of the Cebu library. Understanding how to create, manipulate, and query simplicial complexes is essential for using the library effectively. The examples provided in this document should give you a solid foundation for working with simplicial complexes in Cebu.