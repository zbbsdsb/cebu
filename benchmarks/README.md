# Cebu Benchmarks

This directory contains performance benchmarks for the Cebu library.

## Running Benchmarks

### Prerequisites
- C++20 compatible compiler
- CMake 3.14+
- Built Cebu library

### Build and Run

```bash
# Build the benchmark target
cmake --build build --target benchmark

# Run the benchmarks
cd build && ./benchmark/cebu_benchmark
```

## Benchmark Results

Benchmarks are run on the following hardware:
- **CPU**: AMD Ryzen 9 5950X (16 cores, 32 threads)
- **RAM**: 32GB DDR4-3600
- **Storage**: NVMe SSD
- **OS**: Windows 11 Pro

## Benchmark Types

### Core Operations
- Simplex addition/removal
- Adjacency queries
- Topological invariant calculations

### Spatial Indexing
- BVH tree construction
- Range queries
- Nearest neighbor queries
- Octree performance

### Absurdity System
- Fuzzy interval operations
- Stochastic evolution
- Multi-source fusion

### Serialization
- JSON serialization/deserialization
- Binary serialization/deserialization
- Compression performance

## Interpreting Results

The benchmark output includes:
- Operation name
- Number of simplices
- Average execution time
- Standard deviation
- Operations per second

## CI Integration

Benchmarks run automatically in our CI pipeline on every push to main branch.
The results are stored and can be viewed in the GitHub Actions logs.

## Contributing

To add new benchmarks:
1. Add your benchmark code to `benchmarks/` directory
2. Update `CMakeLists.txt` to include your benchmark
3. Run the benchmarks locally to verify results
4. Submit a PR with your changes

## License

Same as the main Cebu library - MIT License.