# Cebu Library - Advanced Serialization API Documentation

## Overview

This document provides detailed API documentation for the advanced serialization features introduced in Phase 7:
- Change Tracking (ChangeTracker)
- Snapshot Management (SnapshotManager)
- Streaming I/O (StreamingLoader/StreamingWriter)
- Version Control (VersionControl)

---

## Change Tracking

The `change_tracking` namespace provides functionality for tracking changes to simplicial complexes.

### ChangeType Enum

Types of changes that can be tracked:

```cpp
enum class ChangeType {
    SIMPLEX_ADDED,       // A simplex was added to the complex
    SIMPLEX_REMOVED,     // A simplex was removed from the complex
    LABEL_CHANGED,      // A label was modified
    EQUIVALENCE_CHANGED, // An equivalence class was modified
    TOPOLOGY_CHANGED    // The topology structure changed
};
```

### Change Struct

Represents a single change event:

```cpp
struct Change {
    ChangeType type;
    SimplexID simplex_id;
    size_t dimension;
    std::vector<VertexID> vertices;
    std::optional<double> old_label;
    std::optional<double> new_label;
    std::optional<std::string> old_equivalence_class;
    std::optional<std::string> new_equivalence_class;
    std::chrono::system_clock::time_point timestamp;
};
```

### ChangeTracker Class

Tracks all changes made to a simplicial complex.

#### Constructors

```cpp
ChangeTracker();
~ChangeTracker();
```

#### Tracking Methods

```cpp
// Track when a simplex is added
void track_simplex_added(SimplexID id, size_t dimension,
                          const std::vector<VertexID>& vertices);

// Track when a simplex is removed
void track_simplex_removed(SimplexID id, size_t dimension,
                           const std::vector<VertexID>& vertices);

// Track when a label is changed
void track_label_changed(SimplexID id, double old_label, double new_label);

// Track when an equivalence class is changed
void track_equivalence_changed(SimplexID id,
                               const std::string& old_class,
                               const std::string& new_class);

// Track general topology changes
void track_topology_changed();
```

#### Query Methods

```cpp
// Get all changes
std::vector<Change> get_changes() const;

// Get changes by type
std::vector<Change> get_changes_by_type(ChangeType type) const;

// Get changes affecting a specific simplex
std::vector<Change> get_changes_by_simplex(SimplexID id) const;

// Get changes within a time range
std::vector<Change> get_changes_in_time_range(
    const std::chrono::system_clock::time_point& start,
    const std::chrono::system_clock::time_point& end) const;

// Get total change count
size_t get_change_count() const;
```

#### Export/Import Methods

```cpp
// Export changes to JSON
nlohmann::json to_json() const;

// Import changes from JSON
bool from_json(const nlohmann::json& j);

// Save changes to file
bool save_to_file(const std::string& filename) const;

// Load changes from file
bool load_from_file(const std::string& filename);
```

#### Reset Methods

```cpp
// Clear all tracked changes
void reset();

// Check if tracker is empty
bool is_empty() const;
```

#### Usage Example

```cpp
#include "cebu/change_tracker.h"

using namespace cebu;

ChangeTracker tracker;
SimplicialComplexLabeled<double> complex;

// Add vertices and track changes
VertexID v0 = complex.add_vertex();
tracker.track_simplex_added(v0, 0, {v0});

VertexID v1 = complex.add_vertex();
tracker.track_simplex_added(v1, 0, {v1});

// Add edge and track changes
EdgeID e0 = complex.add_edge(v0, v1);
tracker.track_simplex_added(e0, 1, {v0, v1});

// Change label and track changes
complex.set_label(e0, 0.8);
tracker.track_label_changed(e0, 0.0, 0.8);

// Get all changes
auto changes = tracker.get_changes();
std::cout << "Total changes: " << changes.size() << std::endl;

// Save changes to file
tracker.save_to_file("delta.json");
```

---

## Snapshot Management

The `snapshot_management` namespace provides functionality for managing snapshots of simplicial complexes.

### SnapshotMetadata Struct

Metadata about a snapshot:

```cpp
struct SnapshotMetadata {
    std::string name;
    std::chrono::system_clock::time_point timestamp;
    size_t size_bytes;
    size_t simplex_count;
    size_t vertex_count;
    std::string type;
    uint64_t hash;
    bool compressed;
};
```

### Snapshot Struct

A complete snapshot including data:

```cpp
struct Snapshot {
    SnapshotMetadata metadata;
    nlohmann::json data;
};
```

### SnapshotManager Class

Manages multiple snapshots of simplicial complexes.

#### Constructors

```cpp
template<typename LabelType = double>
class SnapshotManager {
public:
    explicit SnapshotManager(const std::string& filename);
    ~SnapshotManager();
};
```

#### Create Methods

```cpp
// Create snapshot from basic complex
bool create_snapshot(const SimplicialComplex& complex,
                     const std::string& name);

// Create snapshot from labeled complex
bool create_snapshot(const SimplicialComplexLabeled<LabelType>& complex,
                     const std::string& name);
```

#### Query Methods

```cpp
// List all snapshot names
std::vector<std::string> list_snapshots() const;

// Check if snapshot exists
bool has_snapshot(const std::string& name) const;

// Get snapshot data
bool get_snapshot(const std::string& name, Snapshot& snapshot) const;

// Get snapshot metadata
SnapshotMetadata get_snapshot_metadata(const std::string& name) const;

// Get snapshot count
size_t get_snapshot_count() const;

// Get total storage size
size_t get_total_size() const;
```

#### Restore Methods

```cpp
// Restore to basic complex
bool restore_snapshot(SimplicialComplex& complex,
                       const std::string& name);

// Restore to labeled complex
bool restore_snapshot(SimplicialComplexLabeled<LabelType>& complex,
                        const std::string& name);
```

#### Delete Methods

```cpp
// Delete a specific snapshot
bool delete_snapshot(const std::string& name);

// Clear all snapshots
void clear_snapshots();
```

#### Compare Methods

```cpp
// Compare two snapshots and return differences
std::vector<Change> compare_snapshots(const std::string& name1,
                                      const std::string& name2) const;
```

#### Compression Methods

```cpp
// Enable/disable compression
void set_compression(bool enable, int level = 6);

// Check compression state
bool is_compression_enabled() const;
int get_compression_level() const;
```

#### Persistence Methods

```cpp
// Load snapshots from file
bool load_from_file();

// Save snapshots to file
bool save_to_file() const;
```

#### Usage Example

```cpp
#include "cebu/snapshot_manager.h"

using namespace cebu;

SnapshotManager manager("snapshots.ceb");
SimplicialComplexLabeled<double> complex;

// Build complex...
VertexID v0 = complex.add_vertex();
VertexID v1 = complex.add_vertex();
complex.add_edge(v0, v1);

// Create snapshot
manager.create_snapshot(complex, "initial");

// Make changes...
complex.set_label(v0, 0.5);

// Create another snapshot
manager.create_snapshot(complex, "checkpoint1");

// List snapshots
auto snapshots = manager.list_snapshots();
for (const auto& name : snapshots) {
    std::cout << "Snapshot: " << name << std::endl;
}

// Compare snapshots
auto changes = manager.compare_snapshots("initial", "checkpoint1");
std::cout << "Changes between snapshots: " << changes.size() << std::endl;

// Restore to initial state
manager.restore_snapshot(complex, "initial");
```

---

## Streaming I/O

The `streaming_io` namespace provides streaming I/O functionality for large files.

### StreamingLoader Class

Loads simplicial complexes using streaming I/O with progress callbacks.

#### Constructors

```cpp
class StreamingLoader {
public:
    explicit StreamingLoader(const std::string& filename);
    ~StreamingLoader();
};
```

#### Load Methods

```cpp
// Load basic complex
std::optional<SimplicialComplex> load();

// Load labeled complex
template<typename LabelType>
std::optional<SimplicialComplexLabeled<LabelType>> load_labeled();
```

#### Metadata Methods

```cpp
// Get file size in bytes
size_t get_file_size() const;

// Get total simplex count
size_t get_total_simplices() const;

// Check if file is compressed
bool is_compressed() const;

// Get file format (binary, json, etc.)
std::string get_file_format() const;

// Get compression information
CompressionInfo get_compression_info() const;
```

#### Progress Callback

```cpp
using ProgressCallback = std::function<void(size_t current, size_t total)>;

void set_progress_callback(ProgressCallback callback);
void clear_progress_callback();
```

#### Chunked Loading

```cpp
// Load a specific chunk of data
template<typename LabelType = double>
std::optional<SimplicialComplexLabeled<LabelType>> load_chunk(
    size_t chunk_index, size_t chunk_size);

// Get total chunk count
size_t get_chunk_count() const;
```

#### Usage Example

```cpp
#include "cebu/streaming_io.h"

using namespace cebu;

// Load large file with progress tracking
StreamingLoader loader("large_complex.json");

// Set progress callback
loader.set_progress_callback([](size_t current, size_t total) {
    double progress = static_cast<double>(current) / total * 100.0;
    std::cout << "Progress: " << progress << "%" << std::endl;
});

// Get metadata
std::cout << "File size: " << loader.get_file_size() << " bytes" << std::endl;
std::cout << "Simplices: " << loader.get_total_simplices() << std::endl;

// Load complex
auto complex_opt = loader.load_labeled<double>();
if (complex_opt) {
    auto& complex = *complex_opt;
    std::cout << "Loaded " << complex.vertex_count() << " vertices" << std::endl;
}
```

### StreamingWriter Class

Writes simplicial complexes using streaming I/O with progress callbacks.

#### Constructors

```cpp
class StreamingWriter {
public:
    explicit StreamingWriter(const std::string& filename);
    ~StreamingWriter();
};
```

#### Write Methods

```cpp
// Write basic complex
bool write(const SimplicialComplex& complex);

// Write labeled complex
template<typename LabelType>
bool write(const SimplicialComplexLabeled<LabelType>& complex);
```

#### Compression Methods

```cpp
// Enable/disable compression
void set_compression(bool enable, int level = 6);
```

#### Progress Callback

```cpp
using ProgressCallback = std::function<void(size_t current, size_t total)>;

void set_progress_callback(ProgressCallback callback);
void clear_progress_callback();
```

#### Metadata Methods

```cpp
// Get bytes written
size_t get_bytes_written() const;

// Get compression information
CompressionInfo get_compression_info() const;
```

#### Usage Example

```cpp
#include "cebu/streaming_io.h"

using namespace cebu;

SimplicialComplexLabeled<double> complex;
// Build complex...

// Write with progress tracking
StreamingWriter writer("output.json");

// Enable compression
writer.set_compression(true, 6);

// Set progress callback
writer.set_progress_callback([](size_t current, size_t total) {
    double progress = static_cast<double>(current) / total * 100.0;
    std::cout << "Writing: " << progress << "%" << std::endl;
});

// Write complex
if (writer.write(complex)) {
    std::cout << "Wrote " << writer.get_bytes_written() << " bytes" << std::endl;
}
```

---

## Version Control

The `version_control` namespace provides Git-style version control for simplicial complexes.

### Types

```cpp
using VersionID = uint64_t;
using BranchID = uint32_t;
using TagID = uint32_t;
```

### VersionMetadata Struct

Metadata about a version:

```cpp
struct VersionMetadata {
    VersionID id;
    std::string message;
    std::string author;
    std::chrono::system_clock::time_point timestamp;
    size_t size_bytes;
    size_t simplex_count;
    std::string type;
    uint64_t hash;
    std::optional<BranchID> branch_id;
};
```

### BranchInfo Struct

Information about a branch:

```cpp
struct BranchInfo {
    BranchID id;
    std::string name;
    VersionID head_version_id;
};
```

### TagInfo Struct

Information about a tag:

```cpp
struct TagInfo {
    TagID id;
    std::string name;
    VersionID version_id;
    std::string description;
};
```

### VersionControl Class

Provides Git-style version control for simplicial complexes.

#### Constructors

```cpp
class VersionControl {
public:
    explicit VersionControl(const std::string& filename);
    ~VersionControl();
};
```

#### Commit Methods

```cpp
// Commit basic complex
VersionID commit(const SimplicialComplex& complex,
                 const std::string& message,
                 const std::string& author);

// Commit labeled complex
template<typename LabelType>
VersionID commit(const SimplicialComplexLabeled<LabelType>& complex,
                 const std::string& message,
                 const std::string& author);
```

#### Checkout Methods

```cpp
// Checkout specific version to basic complex
bool checkout(SimplicialComplex& complex, VersionID version_id);

// Checkout specific version to labeled complex
template<typename LabelType>
bool checkout(SimplicialComplexLabeled<LabelType>& complex,
              VersionID version_id);

// Checkout branch
bool checkout_branch(const std::string& name);
```

#### Branch Management

```cpp
// Create new branch
BranchID create_branch(const std::string& name, VersionID version_id);

// Delete branch
bool delete_branch(const std::string& name);

// Check if branch exists
bool has_branch(const std::string& name) const;

// Get current branch
BranchID get_current_branch() const;

// List all branches
std::vector<BranchInfo> list_branches() const;
```

#### Tag Management

```cpp
// Create tag
TagID create_tag(const std::string& name, VersionID version_id,
                 const std::string& description = "");

// Delete tag
bool delete_tag(const std::string& name);

// Check if tag exists
bool has_tag(const std::string& name) const;

// List all tags
std::vector<std::string> list_tags() const;
```

#### Version Operations

```cpp
// Revert complex to specific version
bool revert(SimplicialComplex& complex, VersionID version_id);

// Get diff between two versions
std::vector<Change> diff(VersionID from_id, VersionID to_id) const;

// Merge branch into current complex
bool merge_branch(SimplicialComplex& complex, const std::string& branch);
```

#### Query Methods

```cpp
// Get commit history
std::vector<VersionMetadata> log(size_t limit = 0) const;

// Get specific version
std::optional<VersionMetadata> get_version(VersionID version_id) const;

// Get HEAD version
VersionID get_head() const;

// Get total version count
size_t get_version_count() const;
```

#### Persistence Methods

```cpp
// Load versions from file
bool load_from_file();

// Save versions to file
bool save_to_file() const;

// Clear all versions
void clear_all();

// Check if empty
bool is_empty() const;
```

#### Usage Example

```cpp
#include "cebu/version_control.h"

using namespace cebu;

VersionControl vc("versions.ceb");
SimplicialComplexLabeled<double> complex;

// Initial state
VertexID v0 = complex.add_vertex();
VertexID v1 = complex.add_vertex();
complex.add_edge(v0, v1);

// Commit initial version
VersionID v1 = vc.commit(complex, "Initial version", "alice");

// Make changes
complex.set_label(v0, 0.5);
VertexID v2 = complex.add_vertex();
complex.add_edge(v1, v2);

// Commit second version
VersionID v2 = vc.commit(complex, "Add vertex", "bob");

// Create branch
vc.create_branch("feature", v1);
vc.checkout_branch("feature");

// Make changes in branch
complex.set_label(v1, 0.8);
VersionID v3 = vc.commit(complex, "Update labels in feature", "charlie");

// Switch back to main
vc.create_branch("main", v2);
vc.checkout_branch("main");

// Merge feature branch
vc.merge_branch(complex, "feature");

// Create tag
vc.create_tag("v1.0.0", v2, "First release");

// View history
auto history = vc.log(10);
for (const auto& meta : history) {
    std::cout << meta.id << ": " << meta.message << std::endl;
}
```

---

## Summary

The advanced serialization features provide:

1. **Change Tracking** - Track all modifications to a complex
2. **Snapshot Management** - Save and restore complex states
3. **Streaming I/O** - Efficient handling of large files
4. **Version Control** - Git-style version management

These features work together to provide a complete serialization and state management system for simplicial complexes.
