cmake_minimum_required(VERSION 3.14.0)
project(riscv-simulator)
include(GNUInstallDirs)
include(ExternalProject)
include(FetchContent)

# Use local find scripts
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# LibELF
find_package(LibElf REQUIRED)

# Compiler config
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_options(-Wall -pedantic)

include_directories(include
                    ${LIBELF_INCLUDE_DIRS})

# Simulator executable
add_executable(rvsim main.cpp
                     HartState.cpp
                     Trace.cpp)
target_link_libraries(rvsim fmt::fmt
                            ${LIBELF_LIBRARIES})

# External dependencies
add_subdirectory(external)

# Tests
enable_testing()
add_subdirectory(tests)
