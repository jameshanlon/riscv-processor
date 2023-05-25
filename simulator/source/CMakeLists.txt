add_library(rvsimlib SHARED
            HartState.cpp
            Trace.cpp)

target_include_directories(rvsimlib PRIVATE
                           ${CMAKE_SOURCE_DIR}/simulator/source
                           ${CMAKE_SOURCE_DIR}/simulator/include)

target_link_libraries(rvsimlib
                      fmt::fmt)
