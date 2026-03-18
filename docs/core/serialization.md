# Basic Serialization

## Overview
Serialization is the process of converting a simplicial complex into a format that can be stored or transmitted, and then reconstructing it later. Cebu provides comprehensive serialization capabilities, including JSON and binary formats, with optional compression support.

## Basic Serialization

### JSON Format
JSON is a human-readable format that's useful for debugging and interoperability:

```cpp
#include "cebu/simplicial_complex.h"
#include "cebu/json_serialization.h"

int main() {
    // Create a simplicial complex
    cebu::SimplicialComplex sc;
    
    // Add some simplices
    auto v1 = sc.add_vertex();
    auto v2 = sc.add_vertex();
    auto v3 = sc.add_vertex();
    sc.add_triangle(v1, v2, v3);
    
    // Serialize to JSON
    std::string json = cebu::serialize_to_json(sc);
    std::cout << json << std::endl;
    
    // Deserialize from JSON
    cebu::SimplicialComplex sc2 = cebu::deserialize_from_json(json);
    
    return 0;
}
```

### Binary Format
Binary format is more compact and faster for large complexes:

```cpp
#include "cebu/simplicial_complex.h"
#include "cebu/serialization.h"

int main() {
    // Create a simplicial complex
    cebu::SimplicialComplex sc;
    
    // Add some simplices
    auto v1 = sc.add_vertex();
    auto v2 = sc.add_vertex();
    auto v3 = sc.add_vertex();
    sc.add_triangle(v1, v2, v3);
    
    // Serialize to binary
    std::vector<uint8_t> binary = cebu::serialize_to_binary(sc);
    std::cout << "Binary size: " << binary.size() << " bytes" << std::endl;
    
    // Deserialize from binary
    cebu::SimplicialComplex sc2 = cebu::deserialize_from_binary(binary);
    
    return 0;
}
```

## Serializing Labeled Complexes

### JSON Format
```cpp
#include "cebu/simplicial_complex_labeled.h"
#include "cebu/json_serialization.h"

int main() {
    // Create a labeled complex
    cebu::SimplicialComplexLabeled<int> sc;
    
    // Add some labeled simplices
    auto v1 = sc.add_vertex(10);
    auto v2 = sc.add_vertex(20);
    auto v3 = sc.add_vertex(30);
    sc.add_triangle(v1, v2, v3, 40);
    
    // Serialize to JSON
    std::string json = cebu::serialize_to_json(sc);
    std::cout << json << std::endl;
    
    // Deserialize from JSON
    cebu::SimplicialComplexLabeled<int> sc2 = cebu::deserialize_from_json<cebu::SimplicialComplexLabeled<int>>(json);
    
    return 0;
}
```

### Binary Format
```cpp
#include "cebu/simplicial_complex_labeled.h"
#include "cebu/serialization.h"

int main() {
    // Create a labeled complex
    cebu::SimplicialComplexLabeled<int> sc;
    
    // Add some labeled simplices
    auto v1 = sc.add_vertex(10);
    auto v2 = sc.add_vertex(20);
    auto v3 = sc.add_vertex(30);
    sc.add_triangle(v1, v2, v3, 40);
    
    // Serialize to binary
    std::vector<uint8_t> binary = cebu::serialize_to_binary(sc);
    std::cout << "Binary size: " << binary.size() << " bytes" << std::endl;
    
    // Deserialize from binary
    cebu::SimplicialComplexLabeled<int> sc2 = cebu::deserialize_from_binary<cebu::SimplicialComplexLabeled<int>>(binary);
    
    return 0;
}
```

## Compression

### ZLIB Compression
Cebu supports ZLIB compression for both JSON and binary formats:

```cpp
#include "cebu/simplicial_complex.h"
#include "cebu/serialization.h"

int main() {
    // Create a complex
    cebu::SimplicialComplex sc;
    // ... add simplices ...
    
    // Serialize to compressed binary
    std::vector<uint8_t> compressed = cebu::serialize_to_compressed_binary(sc);
    std::cout << "Compressed size: " << compressed.size() << " bytes" << std::endl;
    
    // Deserialize from compressed binary
    cebu::SimplicialComplex sc2 = cebu::deserialize_from_compressed_binary(compressed);
    
    return 0;
}
```

## File I/O

### Writing to File
```cpp
#include "cebu/simplicial_complex.h"
#include "cebu/serialization.h"

int main() {
    // Create a complex
    cebu::SimplicialComplex sc;
    // ... add simplices ...
    
    // Write to JSON file
    cebu::write_to_json_file(sc, "complex.json");
    
    // Write to binary file
    cebu::write_to_binary_file(sc, "complex.bin");
    
    // Write to compressed binary file
    cebu::write_to_compressed_binary_file(sc, "complex.bin.gz");
    
    return 0;
}
```

### Reading from File
```cpp
#include "cebu/simplicial_complex.h"
#include "cebu/serialization.h"

int main() {
    // Read from JSON file
    cebu::SimplicialComplex sc1 = cebu::read_from_json_file("complex.json");
    
    // Read from binary file
    cebu::SimplicialComplex sc2 = cebu::read_from_binary_file("complex.bin");
    
    // Read from compressed binary file
    cebu::SimplicialComplex sc3 = cebu::read_from_compressed_binary_file("complex.bin.gz");
    
    return 0;
}
```

## Performance Considerations

### Format Comparison
| Format | Size | Speed (Write) | Speed (Read) | Use Case |
|--------|------|---------------|--------------|----------|
| JSON | Large | Slow | Slow | Debugging, interoperability |
| Binary | Medium | Fast | Fast | Production, large complexes |
| Binary + ZLIB | Small | Slower | Slower | Storage, network传输 |

### Time Complexity
- **Serialization**: O(n), where n is the number of simplices
- **Deserialization**: O(n), where n is the number of simplices

### Space Complexity
- **JSON**: ~10x the size of the complex
- **Binary**: ~2x the size of the complex
- **Binary + ZLIB**: ~0.5x the size of the complex (varies with complexity)

## Best Practices

1. **Choose the Right Format**: Use JSON for debugging, binary for production
2. **Use Compression**: For large complexes or network传输
3. **Batch Operations**: Serialize/deserialize in batches for very large complexes
4. **Error Handling**: Always handle serialization/deserialization errors
5. **Versioning**: Include version information in serialized data

## Examples

### Basic Serialization
```cpp
#include "cebu/simplicial_complex.h"
#include "cebu/json_serialization.h"
#include "cebu/serialization.h"

int main() {
    // Create a complex
    cebu::SimplicialComplex sc;
    
    // Add a tetrahedron
    auto v1 = sc.add_vertex();
    auto v2 = sc.add_vertex();
    auto v3 = sc.add_vertex();
    auto v4 = sc.add_vertex();
    
    sc.add_edge(v1, v2);
    sc.add_edge(v2, v3);
    sc.add_edge(v3, v4);
    sc.add_edge(v4, v1);
    sc.add_edge(v1, v3);
    sc.add_edge(v2, v4);
    
    sc.add_triangle(v1, v2, v3);
    sc.add_triangle(v2, v3, v4);
    sc.add_triangle(v3, v4, v1);
    sc.add_triangle(v4, v1, v2);
    
    sc.add_simplex({v1, v2, v3, v4});
    
    // Serialize to JSON
    std::string json = cebu::serialize_to_json(sc);
    std::cout << "JSON size: " << json.size() << " characters" << std::endl;
    
    // Serialize to binary
    std::vector<uint8_t> binary = cebu::serialize_to_binary(sc);
    std::cout << "Binary size: " << binary.size() << " bytes" << std::endl;
    
    // Serialize to compressed binary
    std::vector<uint8_t> compressed = cebu::serialize_to_compressed_binary(sc);
    std::cout << "Compressed size: " << compressed.size() << " bytes" << std::endl;
    
    // Deserialize
    cebu::SimplicialComplex sc_json = cebu::deserialize_from_json(json);
    cebu::SimplicialComplex sc_binary = cebu::deserialize_from_binary(binary);
    cebu::SimplicialComplex sc_compressed = cebu::deserialize_from_compressed_binary(compressed);
    
    std::cout << "Original vertices: " << sc.get_simplices(0).size() << std::endl;
    std::cout << "JSON vertices: " << sc_json.get_simplices(0).size() << std::endl;
    std::cout << "Binary vertices: " << sc_binary.get_simplices(0).size() << std::endl;
    std::cout << "Compressed vertices: " << sc_compressed.get_simplices(0).size() << std::endl;
    
    return 0;
}
```

### Labeled Complex Serialization
```cpp
#include "cebu/simplicial_complex_labeled.h"
#include "cebu/json_serialization.h"

int main() {
    // Create a labeled complex
    cebu::SimplicialComplexLabeled<std::string> sc;
    
    // Add labeled simplices
    auto v1 = sc.add_vertex("red");
    auto v2 = sc.add_vertex("green");
    auto v3 = sc.add_vertex("blue");
    
    auto e1 = sc.add_edge(v1, v2, "yellow");
    auto e2 = sc.add_edge(v2, v3, "cyan");
    auto e3 = sc.add_edge(v3, v1, "magenta");
    
    auto t1 = sc.add_triangle(v1, v2, v3, "white");
    
    // Serialize to JSON
    std::string json = cebu::serialize_to_json(sc);
    std::cout << json << std::endl;
    
    // Deserialize
    auto sc2 = cebu::deserialize_from_json<cebu::SimplicialComplexLabeled<std::string>>(json);
    
    // Verify labels
    std::cout << "Vertex 0 label: " << sc2.get_label(v1) << std::endl;
    std::cout << "Edge 0 label: " << sc2.get_label(e1) << std::endl;
    std::cout << "Triangle 0 label: " << sc2.get_label(t1) << std::endl;
    
    return 0;
}
```

## Conclusion

Serialization is an essential feature for many applications that use simplicial complexes. Cebu's serialization capabilities provide flexible options for storing and retrieving complex data, whether you need human-readable JSON for debugging or compact binary formats for production. By choosing the right serialization format and following best practices, you can efficiently manage complex data in your applications.