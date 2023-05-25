# fmt
FetchContent_Declare(fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 9.1.0
  GIT_SHALLOW ON
)
FetchContent_MakeAvailable(fmt)

if (fmt_FOUND)
  message(STATUS "Fetched fmt library")
endif()

# Find a RISC-V tool binary
function (find_riscv_tool result_var name)
  find_program(${result_var}
               NAMES ${name}
               PATHS
                 ENV RISCV
               PATH_SUFFIXES bin)
  if (NOT DEFINED ${result_var})
    message("Could not find ${name}")
  endif()
endfunction()

find_riscv_tool(RISCV_UNKNOWN_ELF_GCC riscv32-unknown-elf-gcc)
find_riscv_tool(RISCV_UNKNOWN_ELF_AS riscv32-unknown-elf-as)
find_riscv_tool(RISCV_SPIKE spike)
