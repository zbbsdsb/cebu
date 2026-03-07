cmake_minimum_required(VERSION 3.14)
project(cebu VERSION 0.8.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find and include dependencies
find_package(nlohmann_json 3.2.0 QUIET)

if (NOT nlohmann_json_FOUND)
    # Fetch nlohmann/json if not found
    include(FetchContent)
    set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
    FetchContent_Declare(
        json
        URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(json)
endif()

# Find ZLIB (optional)
find_package(ZLIB QUIET)

if (ZLIB_FOUND)
    add_definitions(-DZLIB_FOUND)
endif()

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Core library
add_library(cebu_core
    src/simplex.cpp
    src/simplicial_complex.cpp
    src/absurdity.cpp
    src/equivalence_classes.cpp
    src/simplicial_complex_non_hausdorff.cpp
    src/refinement.cpp
    src/event_system.cpp
    src/persistence.cpp
    src/json_serialization.cpp
    src/compression.cpp
    src/change_tracker.cpp
    src/snapshot_manager.cpp
    src/streaming_io.cpp
    src/version_control.cpp
    src/vertex_geometry.cpp
    src/bvh.cpp
    src/octree.cpp
    src/parallel_executor.cpp
    src/benchmark.cpp
    src/profiler.cpp
)

# Link nlohmann/json
if (nlohmann_json_FOUND)
    target_link_libraries(cebu_core PRIVATE nlohmann_json::nlohmann_json)
else()
    target_link_libraries(cebu_core PRIVATE nlohmann_json::nlohmann_json)
endif()

# Link ZLIB if found
if (ZLIB_FOUND)
    target_link_libraries(cebu_core PRIVATE ZLIB::ZLIB)
endif()

# Test executable
add_executable(test_basic
    tests/test_basic.cpp
)

target_link_libraries(test_basic cebu_core)

# Dynamic test executable
add_executable(test_dynamic
    tests/test_dynamic.cpp
)

target_link_libraries(test_dynamic cebu_core)

# Label test executable
add_executable(test_labels
    tests/test_labels.cpp
)

target_link_libraries(test_labels cebu_core)

# Story events test executable
add_executable(test_story_events
    tests/test_story_events.cpp
)

target_link_libraries(test_story_events cebu_core)

# Timeline test executable
add_executable(test_timeline
    tests/test_timeline.cpp
)

target_link_libraries(test_timeline cebu_core)

# Topology operations test executable
add_executable(test_topology_ops
    tests/test_topology_ops.cpp
)

target_link_libraries(test_topology_ops cebu_core)

# Narrative test executable
add_executable(test_narrative
    tests/test_narrative.cpp
)

target_link_libraries(test_narrative cebu_core)

# Topology operations advanced test executable
add_executable(test_topology_ops_advanced
    tests/test_topology_ops_advanced.cpp
)

target_link_libraries(test_topology_ops_advanced cebu_core)

# Command history test executable
add_executable(test_command_history
    tests/test_command_history.cpp
)

target_link_libraries(test_command_history cebu_core)

# Serialization test executable
add_executable(test_serialization
    tests/test_serialization.cpp
)

target_link_libraries(test_serialization cebu_core)

# Equivalence classes test executable
add_executable(test_equivalence_classes
    tests/test_equivalence_classes.cpp
)

target_link_libraries(test_equivalence_classes cebu_core)

# Non-Hausdorff test executable
add_executable(test_non_hausdorff
    tests/test_non_hausdorff.cpp
)

target_link_libraries(test_non_hausdorff cebu_core)

# Refinement test executable
add_executable(test_refinement
    tests/test_refinement.cpp
)

target_link_libraries(test_refinement cebu_core)

# Binary serialization test executable
add_executable(test_binary_serialization
    tests/test_binary_serialization.cpp
)

target_link_libraries(test_binary_serialization cebu_core)

# Event system test executable
add_executable(test_event_system
    tests/test_event_system.cpp
)

target_link_libraries(test_event_system cebu_core)

# Persistence test executable
add_executable(test_persistence
    tests/test_persistence.cpp
)

target_link_libraries(test_persistence cebu_core)

# JSON serialization test executable
add_executable(test_json_serialization
    tests/test_json_serialization.cpp
)

target_link_libraries(test_json_serialization cebu_core)

# Compression test executable
add_executable(test_compression
    tests/test_compression.cpp
)

target_link_libraries(test_compression cebu_core)

# Non-Hausdorff serialization test executable
add_executable(test_non_hausdorff_serialization
    tests/test_non_hausdorff_serialization.cpp
)

target_link_libraries(test_non_hausdorff_serialization cebu_core)

# Change tracker test executable
add_executable(test_change_tracker
    tests/test_change_tracker.cpp
)

target_link_libraries(test_change_tracker cebu_core)

# Snapshot manager test executable
add_executable(test_snapshot_manager
    tests/test_snapshot_manager.cpp
)

target_link_libraries(test_snapshot_manager cebu_core)

# Streaming I/O test executable
add_executable(test_streaming_io
    tests/test_streaming_io.cpp
)

target_link_libraries(test_streaming_io cebu_core)

# Version control test executable
add_executable(test_version_control
    tests/test_version_control.cpp
)

target_link_libraries(test_version_control cebu_core)

# Vertex geometry test executable
add_executable(test_vertex_geometry
    tests/test_vertex_geometry.cpp
)

target_link_libraries(test_vertex_geometry cebu_core)

# BVH test executable
add_executable(test_bvh
    tests/test_bvh.cpp
)

target_link_libraries(test_bvh cebu_core)

# Octree test executable
add_executable(test_octree
    tests/test_octree.cpp
)

target_link_libraries(test_octree cebu_core)

# Profiler test executable
add_executable(test_profiler
    tests/test_profiler.cpp
)

target_link_libraries(test_profiler cebu_core)

# Phase 9a: Absurdity test executable
add_executable(test_phase9a_absurdity
    tests/test_phase9a_absurdity.cpp
)

target_link_libraries(test_phase9a_absurdity cebu_core)

# Compilation test executables
add_executable(test_vertex_geometry_compilation
    tests/test_vertex_geometry_compilation.cpp
)

target_link_libraries(test_vertex_geometry_compilation cebu_core)

add_executable(test_bvh_compilation
    tests/test_bvh_compilation.cpp
)

target_link_libraries(test_bvh_compilation cebu_core)

add_executable(test_octree_compilation
    tests/test_octree_compilation.cpp
)

target_link_libraries(test_octree_compilation cebu_core)

# Parallel executor test executable
add_executable(test_parallel_executor
    tests/test_parallel_executor.cpp
)

target_link_libraries(test_parallel_executor cebu_core)

# Phase 8c demo executable
add_executable(phase8c_demo
    examples/phase8c_demo.cpp
)

target_link_libraries(phase8c_demo cebu_core)

# Phase 8d benchmark executable
add_executable(phase8d_benchmark
    examples/phase8d_benchmark.cpp
)

target_link_libraries(phase8d_benchmark cebu_core)

# Phase 9a demo executable
add_executable(phase9a_demo
    examples/phase9a_demo.cpp
)

target_link_libraries(phase9a_demo cebu_core)

# Phase 9a minimal test executable
add_executable(phase9a_minimal_test
    examples/phase9a_minimal_test.cpp
)

target_link_libraries(phase9a_minimal_test cebu_core)

# Phase 9b morph test executable
add_executable(test_phase9b_morph
    tests/test_phase9b_morph.cpp
)

target_link_libraries(test_phase9b_morph cebu_core)

# Phase 9b demo executable
add_executable(phase9b_demo
    examples/phase9b_demo.cpp
)

target_link_libraries(phase9b_demo cebu_core)

# Phase 9c narrative test executable
add_executable(test_phase9c_narrative
    tests/test_phase9c_narrative.cpp
)

target_link_libraries(test_phase9c_narrative cebu_core)

# Phase 9c demo executable
add_executable(phase9c_demo
    examples/phase9c_demo.cpp
)

target_link_libraries(phase9c_demo cebu_core)

# Enable testing
enable_testing()
add_test(NAME basic_test COMMAND test_basic)
add_test(NAME dynamic_test COMMAND test_dynamic)
add_test(NAME labels_test COMMAND test_labels)
add_test(NAME story_events_test COMMAND test_story_events)
add_test(NAME timeline_test COMMAND test_timeline)
add_test(NAME topology_ops_test COMMAND test_topology_ops)
add_test(NAME narrative_test COMMAND test_narrative)
add_test(NAME topology_ops_advanced_test COMMAND test_topology_ops_advanced)
add_test(NAME command_history_test COMMAND test_command_history)
add_test(NAME serialization_test COMMAND test_serialization)
add_test(NAME equivalence_classes_test COMMAND test_equivalence_classes)
add_test(NAME non_hausdorff_test COMMAND test_non_hausdorff)
add_test(NAME refinement_test COMMAND test_refinement)
add_test(NAME binary_serialization_test COMMAND test_binary_serialization)
add_test(NAME event_system_test COMMAND test_event_system)
add_test(NAME persistence_test COMMAND test_persistence)
add_test(NAME json_serialization_test COMMAND test_json_serialization)
add_test(NAME compression_test COMMAND test_compression)
add_test(NAME non_hausdorff_serialization_test COMMAND test_non_hausdorff_serialization)
add_test(NAME change_tracker_test COMMAND test_change_tracker)
add_test(NAME snapshot_manager_test COMMAND test_snapshot_manager)
add_test(NAME streaming_io_test COMMAND test_streaming_io)
add_test(NAME version_control_test COMMAND test_version_control)
add_test(NAME vertex_geometry_test COMMAND test_vertex_geometry)
add_test(NAME bvh_test COMMAND test_bvh)
add_test(NAME octree_test COMMAND test_octree)
