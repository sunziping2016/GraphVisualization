cmake_minimum_required(VERSION 3.6)
project(GraphVisualization)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -O2")

set(LIB_SOURCE_FILES src/export.cpp src/export.h src/graph.h src/graph.cpp)
add_library(libgraph SHARED ${LIB_SOURCE_FILES})

set(TEST_SOURCE_FILES src/graph.h src/graph.cpp src/test.cpp)
add_executable(graph-test ${TEST_SOURCE_FILES})

set_target_properties(libgraph PROPERTIES
        OUTPUT_NAME "graph"
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/gui)
