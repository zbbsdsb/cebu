cmake_minimum_required(VERSION 3.14)
project(cebu VERSION 0.4.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
