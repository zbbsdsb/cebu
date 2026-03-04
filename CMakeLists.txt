cmake_minimum_required(VERSION 3.14)
project(cebu VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Core library
add_library(cebu_core
    src/simplex.cpp
    src/simplicial_complex.cpp
    src/absurdity.cpp
)

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

# Enable testing
enable_testing()
add_test(NAME basic_test COMMAND test_basic)
add_test(NAME dynamic_test COMMAND test_dynamic)
add_test(NAME labels_test COMMAND test_labels)
add_test(NAME story_events_test COMMAND test_story_events)
add_test(NAME timeline_test COMMAND test_timeline)
add_test(NAME topology_ops_test COMMAND test_topology_ops)
