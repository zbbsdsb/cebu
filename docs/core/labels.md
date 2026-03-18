# Label System

## Overview
The Cebu library provides a powerful label system that allows you to attach custom data to simplices. Labels can be used to store additional information such as colors, weights, material properties, or any other data relevant to your application.

## Labeled Simplicial Complexes

### Basic Usage
```cpp
#include "cebu/simplicial_complex_labeled.h"

// Create a labeled simplicial complex with int labels
cebu::SimplicialComplexLabeled<int> sc;

// Add vertices with labels
auto v1 = sc.add_vertex(10);
auto v2 = sc.add_vertex(20);
auto v3 = sc.add_vertex(30);

// Add an edge with a label
auto e1 = sc.add_edge(v1, v2, 15);

// Add a triangle with a label
auto t1 = sc.add_triangle(v1, v2, v3, 25);
```

### Template Parameters
The `SimplicialComplexLabeled` class is a template that takes the label type as a parameter:

```cpp
// With different label types
cebu::SimplicialComplexLabeled<float> sc_float;        // Float labels
cebu::SimplicialComplexLabeled<std::string> sc_string;  // String labels
cebu::SimplicialComplexLabeled<MyStruct> sc_custom;    // Custom struct labels
```

### Requirements for Label Types
Label types must be:
- Copyable
- Default constructible (for some operations)

## Label Operations

### Setting Labels
```cpp
// Set label for a simplex
sc.set_label(v1, 100);
sc.set_label(e1, 200);
sc.set_label(t1, 300);
```

### Getting Labels
```cpp
// Get label for a simplex
int v_label = sc.get_label(v1);
int e_label = sc.get_label(e1);
int t_label = sc.get_label(t1);
```

### Querying by Label

#### Numeric Queries
```cpp
// Get simplices with label greater than 20
auto high_labeled = sc.get_high_labeled_simplices(20);

// Get simplices with label less than 20
auto low_labeled = sc.get_low_labeled_simplices(20);

// Get simplices with label between 15 and 25
auto range_labeled = sc.get_range_labeled_simplices(15, 25);
```

#### Predicate Queries
```cpp
// Get simplices with even labels
auto even_labeled = sc.get_labeled_simplices([](int label) {
    return label % 2 == 0;
});
```

#### Dimension-Specific Queries
```cpp
// Get vertices with label greater than 20
auto high_vertices = sc.get_high_labeled_simplices(0, 20);

// Get edges with label less than 20
auto low_edges = sc.get_low_labeled_simplices(1, 20);
```

## Advanced Labeling

### Absurdity Labels
For narrative applications, Cebu provides interval-valued fuzzy labels:

```cpp
#include "cebu/absurdity.h"

// Create a complex with absurdity labels
cebu::SimplicialComplexLabeled<cebu::Absurdity> sc;

// Add vertices with absurdity labels
auto v1 = sc.add_vertex(cebu::Absurdity(0.0, 0.2));
auto v2 = sc.add_vertex(cebu::Absurdity(0.5, 0.7));

// Add an edge with an absurdity label
auto e1 = sc.add_edge(v1, v2, cebu::Absurdity(0.3, 0.5));
```

### Label Inheritance
When refining meshes, labels can be inherited by child simplices:

```cpp
#include "cebu/refinement.h"

// Create a refinement strategy with label inheritance
cebu::RefinementStrategy strategy;
strategy.set_label_inheritance(cebu::LabelInheritance::INTERPOLATE);

// Refine a simplex
auto refined = strategy.refine_edge(sc, e1);
```

## Performance Considerations

### Time Complexity
- **Setting a label**: O(1)
- **Getting a label**: O(1)
- **Querying by label**: O(n), where n is the number of simplices

### Space Complexity
- Additional O(n) space for labels, where n is the number of simplices

## Best Practices

1. **Choose Appropriate Label Types**: Use the smallest possible type for your labels to save memory
2. **Batch Operations**: For large complexes, use batch operations to set labels
3. **Spatial Indexing**: Combine labels with spatial indexing for efficient queries
4. **Custom Predicates**: Use custom predicates for complex label queries

## Examples

### Basic Labeling
```cpp
#include "cebu/simplicial_complex_labeled.h"

int main() {
    // Create a labeled complex
    cebu::SimplicialComplexLabeled<float> sc;
    
    // Add vertices with weights
    auto v1 = sc.add_vertex(1.0f);
    auto v2 = sc.add_vertex(2.0f);
    auto v3 = sc.add_vertex(3.0f);
    
    // Add edges with weights
    auto e1 = sc.add_edge(v1, v2, 1.5f);
    auto e2 = sc.add_edge(v2, v3, 2.5f);
    auto e3 = sc.add_edge(v3, v1, 2.0f);
    
    // Add a triangle with weight
    auto t1 = sc.add_triangle(v1, v2, v3, 2.0f);
    
    // Query by weight
    auto heavy_vertices = sc.get_high_labeled_simplices(0, 2.0f);
    auto light_edges = sc.get_low_labeled_simplices(1, 2.0f);
    
    std::cout << "Heavy vertices: " << heavy_vertices.size() << std::endl;
    std::cout << "Light edges: " << light_edges.size() << std::endl;
    
    return 0;
}
```

### Custom Label Types
```cpp
#include "cebu/simplicial_complex_labeled.h"

// Custom label struct
struct VertexProperties {
    float x, y, z;      // Position
    int material_id;     // Material ID
    bool is_boundary;    // Boundary flag
};

int main() {
    // Create a complex with custom labels
    cebu::SimplicialComplexLabeled<VertexProperties> sc;
    
    // Add vertices with properties
    auto v1 = sc.add_vertex({0.0f, 0.0f, 0.0f, 1, false});
    auto v2 = sc.add_vertex({1.0f, 0.0f, 0.0f, 1, true});
    auto v3 = sc.add_vertex({0.0f, 1.0f, 0.0f, 2, false});
    
    // Get boundary vertices
    auto boundary_vertices = sc.get_labeled_simplices([](const VertexProperties& prop) {
        return prop.is_boundary;
    });
    
    std::cout << "Boundary vertices: " << boundary_vertices.size() << std::endl;
    
    return 0;
}
```

### Absurdity Labels for Narrative
```cpp
#include "cebu/simplicial_complex_labeled.h"
#include "cebu/absurdity.h"

int main() {
    // Create a complex with absurdity labels
    cebu::SimplicialComplexLabeled<cebu::Absurdity> sc;
    
    // Add vertices with absurdity values
    auto v1 = sc.add_vertex(cebu::Absurdity(0.0, 0.1)); // Low absurdity
    auto v2 = sc.add_vertex(cebu::Absurdity(0.4, 0.6)); // Medium absurdity
    auto v3 = sc.add_vertex(cebu::Absurdity(0.9, 1.0)); // High absurdity
    
    // Add edges
    auto e1 = sc.add_edge(v1, v2, cebu::Absurdity(0.2, 0.4));
    auto e2 = sc.add_edge(v2, v3, cebu::Absurdity(0.6, 0.8));
    auto e3 = sc.add_edge(v3, v1, cebu::Absurdity(0.5, 0.7));
    
    // Get high absurdity simplices
    auto high_absurdity = sc.get_labeled_simplices([](const cebu::Absurdity& abs) {
        return abs.mean() > 0.7;
    });
    
    std::cout << "High absurdity simplices: " << high_absurdity.size() << std::endl;
    
    return 0;
}
```

## Conclusion

The label system in Cebu provides a flexible way to attach custom data to simplices. Whether you're storing simple numeric values or complex custom structures, the label system can accommodate your needs. By combining labels with other features like spatial indexing and mesh refinement, you can create powerful and efficient topological models for your applications.