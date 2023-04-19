# Attempt to find libelf.

find_path(LIBELF_INCLUDE_DIRS
  NAMES
    libelf.h
  PATHS
    /usr/include
    /usr/include/libelf
    /usr/local/include
    /usr/local/include/libelf
    /opt/local/include
    /opt/local/include/libelf
  ENV CPATH)

# If libelf.h is found in a 'libelf/' directory, also include the parent level
# as a search path since other ELF headers reside here on some platforms.
if (LIBELF_INCLUDE_DIRS MATCHES "libelf$")
  set(LIBELF_INCLUDE_DIRS "${LIBELF_INCLUDE_DIRS}"
                          "${LIBELF_INCLUDE_DIRS}/.."
      CACHE STRING "" FORCE)
endif()

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
