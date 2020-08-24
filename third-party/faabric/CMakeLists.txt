cmake_minimum_required(VERSION 3.13.0)
project(faabric)

# Top-level CMake config
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(FAABRIC_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)

# Include directories
include_directories(
        ${FAABRIC_INCLUDE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/src # Required for protobuf
)

add_subdirectory(src/proto)
add_subdirectory(src/redis)
add_subdirectory(src/scheduler)
add_subdirectory(src/state)
add_subdirectory(src/util)

add_library(faabric faabric.cpp)

target_link_libraries(faabric
        proto
        redis
        scheduler
        state
        util
        )

# Tests
add_library(catch INTERFACE)
add_subdirectory(tests/test)
add_subdirectory(tests/utils)
