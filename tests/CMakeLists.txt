# Catch2
FetchContent_Declare(Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.3.2
  GIT_SHALLOW ON
)
FetchContent_MakeAvailable(Catch2)

if (Catch2_FOUND)
  message(STATUS "Fetched Catch2 library")
endif()

# C++ unit tests
add_executable(tests
               main.cpp
               tests.cpp)

target_link_libraries(tests PRIVATE Catch2::Catch2
                                    fmt::fmt)

add_test(NAME tests COMMAND tests)

# Python unit tests
configure_file(config.py.in ${CMAKE_CURRENT_BINARY_DIR}/config.py)
configure_file(tests.py ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

add_test(NAME tests.py
         COMMAND ${_Python_EXECUTABLE} tests.py)

# Riscof
add_subdirectory(riscof)
