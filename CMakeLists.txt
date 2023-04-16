cmake_minimum_required(VERSION 3.14.0)
project(riscv-simulator)
include(GNUInstallDirs)
include(ExternalProject)

# Use local find scripts
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

find_package(LibElf REQUIRED)

# Options
option(BUILD_GNU_TOOLCHAIN "Build a RISC-V GNU toolchain"   OFF)
option(BUILD_PK            "Build the RISC-V proxy kernel"  OFF)
option(BUILD_SPIKE         "Build the RISC-V ISA simulator" OFF)

set(NUM_PARALLEL_JOBS 16)

# Compiler config
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_options(-Wall -pedantic)

include_directories(${LIBELF_INCLUDE_DIRS})

# Simulator executable
add_executable(rvsim rvsim.cpp)
target_link_libraries(rvsim ${LIBELF_LIBRARIES})

if (BUILD_GNU_TOOLCHAIN)
  ExternalProject_Add(riscv-gnu-toolchain
    GIT_REPOSITORY https://github.com/riscv-collab/riscv-gnu-toolchain.git
    GIT_SHALLOW ON
    INSTALL_DIR ${CMAKE_BINARY_DIR}/riscv
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=<INSTALL_DIR> --with-arch=rv32im
    BUILD_COMMAND make -j ${NUM_PARALLEL_JOBS}
    )
  add_dependencies(rvsim riscv-gnu-toolchain)
endif()

if (BUILD_PK)
  ExternalProject_Add(riscv-pk
    GIT_REPOSITORY https://github.com/riscv-software-src/riscv-pk.git
    GIT_SHALLOW ON
    INSTALL_DIR ${CMAKE_BINARY_DIR}/riscv
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=<INSTALL_DIR> --host=riscv32-unknown-elf
    BUILD_COMMAND make -j ${NUM_PARALLEL_JOBS}
    INSTALL_COMMAND make install
    )
  add_dependencies(rvsim riscv-pk)
endif()

if (BUILD_SPIKE)
  ExternalProject_Add(riscv-isa-sim
    GIT_REPOSITORY https://github.com/riscv-software-src/riscv-isa-sim.git
    GIT_SHALLOW ON
    INSTALL_DIR ${CMAKE_BINARY_DIR}/riscv
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=<INSTALL_DIR> --with-target=riscv32-unknown-elf
    BUILD_COMMAND make -j ${NUM_PARALLEL_JOBS}
    INSTALL_COMMAND make install
    )
  add_dependencies(rvsim riscv-isa-sim)
endif()
