# Attempt to find libelf.

find_path(LIBELF_INCLUDE_DIRS
  NAMES
    libelf/libelf.h
  PATHS
    /usr/include
    /usr/local/include
    /opt/local/include
    /sw/include
  ENV CPATH)

# Include the 'libelf' prefix for include paths.
set(LIBELF_INCLUDE_DIRS
    "${LIBELF_INCLUDE_DIRS}" "${LIBELF_INCLUDE_DIRS}/libelf"
    CACHE STRING "" FORCE)

find_library(LIBELF_LIBRARIES
  NAMES
    elf
  PATHS
    /usr/lib
    /usr/local/lib
    /opt/local/lib
    /sw/lib
  ENV LIBRARY_PATH
  ENV LD_LIBRARY_PATH)

include(FindPackageHandleStandardArgs)

# handle the QUIETLY and REQUIRED arguments and set LIBELF_FOUND to TRUE if
# all listed variables are TRUE.
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibElf DEFAULT_MSG
  LIBELF_LIBRARIES
  LIBELF_INCLUDE_DIRS)
