add_executable(rvsim main.cpp)

target_include_directories(rvsim PRIVATE
                           ${CMAKE_SOURCE_DIR}/simulator/include
                           ${LIBELF_INCLUDE_DIRS})

target_link_libraries(rvsim
                      rvsimlib
                      fmt::fmt
                      ${LIBELF_LIBRARIES})
