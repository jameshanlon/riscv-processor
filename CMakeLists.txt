cmake_minimum_required(VERSION 3.14.0)
project(riscv-simulator)
include(GNUInstallDirs)

# Use local find scripts
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

find_package(LibElf REQUIRED)

# Compiler config
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_options(-Wall -pedantic)

include_directories(${LIBELF_INCLUDE_DIRS})

# Simulator executable
add_executable(rvsim rvsim.cpp)
target_link_libraries(rvsim ${LIBELF_LIBRARIES})
