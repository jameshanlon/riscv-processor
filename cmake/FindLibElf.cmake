# Attempt to find libelf headers and library.
# This module defines:
#  LIBELF_FOUND
#  LIBELF_INCLUDE_DIRS
#  LIBELF_LIBRARIES

find_path(LIBELF_INCLUDE_DIR
          NAMES libelf.h
          PATH_SUFFIXES libelf)

# If libelf.h is found in a 'libelf/' directory, also include the parent level
# as a search path since other ELF headers reside here on some platforms.
if (LIBELF_INCLUDE_DIR MATCHES "libelf$")
  string(REGEX REPLACE "libelf$" ""
           LILBELF_INCLIDE_DIRS
           "${LIBELF_INCLUDE_DIR}")
  set(LIBELF_INCLUDE_DIRS
       "${LIBELF_INCLUDE_DIRS}"
       "${LIBELF_INCLUDE_DIR}"
      CACHE STRING "" FORCE)
else()
  set(LIBELF_INCLUDE_DIRS "${LIBELF_INCLUDE_DIR}")
endif()

find_library(LIBELF_LIBRARIES
             NAMES elf)

include(FindPackageHandleStandardArgs)

# handle the QUIETLY and REQUIRED arguments and set LIBELF_FOUND to TRUE if
# all listed variables are TRUE.
find_package_handle_standard_args(LibElf DEFAULT_MSG
  LIBELF_LIBRARIES
  LIBELF_INCLUDE_DIRS)
