cmake_minimum_required(VERSION 3.14)
project(test_cebu_conan)

find_package(cebu CONFIG REQUIRED)

add_executable(test_cebu_conan main.cpp)
target_link_libraries(test_cebu_conan cebu::cebu)