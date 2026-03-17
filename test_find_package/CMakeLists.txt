cmake_minimum_required(VERSION 3.14)
project(test_cebu_find_package)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 尝试找到Cebu库
find_package(cebu CONFIG REQUIRED)

# 创建测试可执行文件
add_executable(test_find_package main.cpp)

# 链接Cebu库
target_link_libraries(test_find_package PRIVATE cebu::cebu_core)
