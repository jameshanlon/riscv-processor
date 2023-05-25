cmake_minimum_required(VERSION 3.14.0)
project(riscv-processor)
include(GNUInstallDirs)
include(ExternalProject)
include(FetchContent)

# Use local find scripts
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# Compiler config
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_options(-Wall -pedantic)

add_subdirectory(external)
add_subdirectory(simulator)

# Tests
enable_testing()
add_subdirectory(tests)
