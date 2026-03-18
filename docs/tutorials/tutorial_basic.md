# Basic Usage Tutorial

## Overview
This tutorial will guide you through the basic usage of the Cebu library, from installation to creating your first simplicial complex.

## Prerequisites

### Installation
Before starting, make sure you have Cebu installed. For installation instructions, see the [Installation Guide](../installation.md).

### Required Headers
For basic usage, you'll need to include the following headers:

```cpp
#include "cebu/simplicial_complex.h"
```

## Creating Your First Simplicial Complex

### Step 1: Create a Simple Complex
Let's start by creating a basic simplicial complex with a few vertices and edges:

```cpp
#include "cebu/simplicial_complex.h"
#include <iostream>

int main() {
    // Create a simplicial complex
    cebu::SimplicialComplex sc;
    
    // Add vertices
    auto v1 = sc.add_vertex();
    auto v2 = sc.add_vertex();
    auto v3 = sc.add_vertex();
    
    // Add edges
    auto e1 = sc.add_edge(v1, v2);
    auto e2 = sc.add_edge(v2, v3);
    auto e3 = sc.add_edge(v3, v1);
    
    // Print information about the complex
    std::cout << "Vertices: " << sc.get_simplices(0).size() << std::endl;
    std::cout << "Edges: " << sc.get_simplices(1).size() << std::endl;
    std::cout << "Triangles: " << sc.get_simplices(2).size() << std::endl;
    
    return 0;
}
```

### Step 2: Add a Triangle
Now let's add a triangle to our complex:

```cpp
// Add a triangle
auto t1 = sc.add_triangle(v1, v2, v3);

// Print updated information
std::cout << "Triangles: " << sc.get_simplices(2).size() << std::endl;
```

### Step 3: Query Simplices
Let's query the simplices in our complex:

```cpp
// Get all vertices
auto vertices = sc.get_simplices(0);
std::cout << "Vertices: ";
for (auto v : vertices) {
    std::cout << v << " ";
}
std::cout << std::endl;

// Get all edges incident to v1
auto incident_edges = sc.get_incident_edges(v1);
std::cout << "Edges incident to v1: ";
for (auto e : incident_edges) {
    std::cout << e << " ";
}
std::cout << std::endl;
```

### Step 4: Remove a Simplex
Let's remove an edge and see how it affects the complex:

```cpp
// Remove edge e1
sc.remove_edge(e1);

// Print updated information
std::cout << "Edges: " << sc.get_simplices(1).size() << std::endl;
std::cout << "Triangles: " << sc.get_simplices(2).size() << std::endl;
```

Notice that when we remove edge e1, the triangle t1 is also removed because it depends on e1.

## Working with Labeled Complexes

### Step 1: Create a Labeled Complex
Cebu allows you to attach labels to simplices. Let's create a labeled complex:

```cpp
#include "cebu/simplicial_complex_labeled.h"

int main() {
    // Create a labeled complex with int labels
    cebu::SimplicialComplexLabeled<int> sc;
    
    // Add vertices with labels
    auto v1 = sc.add_vertex(10);
    auto v2 = sc.add_vertex(20);
    auto v3 = sc.add_vertex(30);
    
    // Add edges with labels
    auto e1 = sc.add_edge(v1, v2, 15);
    auto e2 = sc.add_edge(v2, v3, 25);
    auto e3 = sc.add_edge(v3, v1, 20);
    
    // Add a triangle with a label
    auto t1 = sc.add_triangle(v1, v2, v3, 20);
    
    return 0;
}
```

### Step 2: Query by Label
Let's query simplices based on their labels:

```cpp
// Get vertices with label greater than 15
auto high_vertices = sc.get_high_labeled_simplices(0, 15);
std::cout << "Vertices with label > 15: ";
for (auto v : high_vertices) {
    std::cout << v << " (label: " << sc.get_label(v) << ") ";
}
std::cout << std::endl;

// Get edges with label between 10 and 25
auto range_edges = sc.get_range_labeled_simplices(1, 10, 25);
std::cout << "Edges with label between 10 and 25: ";
for (auto e : range_edges) {
    std::cout << e << " (label: " << sc.get_label(e) << ") ";
}
std::cout << std::endl;
```

### Step 3: Update Labels
Let's update the label of a simplex:

```cpp
// Update the label of v1
sc.set_label(v1, 100);
std::cout << "New label for v1: " << sc.get_label(v1) << std::endl;
```

## Serialization

### Step 1: Serialize to JSON
Let's serialize our complex to JSON format:

```cpp
#include "cebu/json_serialization.h"

// Serialize to JSON
std::string json = cebu::serialize_to_json(sc);
std::cout << "JSON representation: " << json << std::endl;
```

### Step 2: Deserialize from JSON
Now let's deserialize the JSON back into a complex:

```cpp
// Deserialize from JSON
cebu::SimplicialComplexLabeled<int> sc2 = cebu::deserialize_from_json<cebu::SimplicialComplexLabeled<int>>(json);

// Verify the deserialized complex
std::cout << "Deserialized vertices: " << sc2.get_simplices(0).size() << std::endl;
std::cout << "Deserialized edges: " << sc2.get_simplices(1).size() << std::endl;
std::cout << "Deserialized triangles: " << sc2.get_simplices(2).size() << std::endl;
```

## Complete Example

Here's a complete example that demonstrates all the concepts we've covered:

```cpp
#include "cebu/simplicial_complex_labeled.h"
#include "cebu/json_serialization.h"
#include <iostream>

int main() {
    // Create a labeled complex
    cebu::SimplicialComplexLabeled<int> sc;
    
    // Add vertices with labels
    auto v1 = sc.add_vertex(10);
    auto v2 = sc.add_vertex(20);
    auto v3 = sc.add_vertex(30);
    auto v4 = sc.add_vertex(40);
    
    // Add edges with labels
    auto e1 = sc.add_edge(v1, v2, 15);
    auto e2 = sc.add_edge(v2, v3, 25);
    auto e3 = sc.add_edge(v3, v4, 35);
    auto e4 = sc.add_edge(v4, v1, 25);
    auto e5 = sc.add_edge(v1, v3, 20);
    
    // Add triangles with labels
    auto t1 = sc.add_triangle(v1, v2, v3, 20);
    auto t2 = sc.add_triangle(v1, v3, v4, 30);
    
    // Print initial state
    std::cout << "Initial state:" << std::endl;
    std::cout << "Vertices: " << sc.get_simplices(0).size() << std::endl;
    std::cout << "Edges: " << sc.get_simplices(1).size() << std::endl;
    std::cout << "Triangles: " << sc.get_simplices(2).size() << std::endl;
    
    // Query by label
    auto high_vertices = sc.get_high_labeled_simplices(0, 25);
    std::cout << "\nVertices with label > 25: ";
    for (auto v : high_vertices) {
        std::cout << v << " (label: " << sc.get_label(v) << ") ";
    }
    std::cout << std::endl;
    
    // Remove an edge
    sc.remove_edge(e5);
    std::cout << "\nAfter removing edge e5:" << std::endl;
    std::cout << "Edges: " << sc.get_simplices(1).size() << std::endl;
    std::cout << "Triangles: " << sc.get_simplices(2).size() << std::endl;
    
    // Serialize to JSON
    std::string json = cebu::serialize_to_json(sc);
    std::cout << "\nJSON size: " << json.size() << " characters" << std::endl;
    
    // Deserialize from JSON
    cebu::SimplicialComplexLabeled<int> sc2 = cebu::deserialize_from_json<cebu::SimplicialComplexLabeled<int>>(json);
    std::cout << "\nDeserialized complex:" << std::endl;
    std::cout << "Vertices: " << sc2.get_simplices(0).size() << std::endl;
    std::cout << "Edges: " << sc2.get_simplices(1).size() << std::endl;
    std::cout << "Triangles: " << sc2.get_simplices(2).size() << std::endl;
    
    return 0;
}
```

## Next Steps

Now that you've learned the basics of Cebu, you can explore more advanced features:

1. **Spatial Indexing**: Learn how to use BVH and Octree for fast spatial queries
2. **Topology Operations**: Explore topological invariants and operations
3. **Advanced Serialization**: Learn about version control, snapshots, and streaming I/O
4. **Non-Hausdorff Topology**: Explore simplex gluing and equivalence classes

For more information, see the [Core Concepts](../core/) and [Advanced Features](../advanced/) sections of the documentation.