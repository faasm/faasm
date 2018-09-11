cmake_minimum_required(VERSION 3.0)
project(faasm)

SET(CMAKE_CXX_FLAGS "-Wall -Werror")
SET(CMAKE_CXX_FLAGS_DEBUG "-g")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

enable_testing()

# Enable asan in debug builds.
if (CMAKE_BUILD_TYPE MATCHES "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
endif ()

# Stops WAVM compilation complaining
add_definitions(-DDLL_EXPORT=)
add_definitions(-DDLL_IMPORT=)

set(FAASM_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(FAASM_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
set(FAASM_WAVM_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/WAVM/Lib)
set(FAASM_WAVM_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/WAVM/Include)

include_directories(
        ${FAASM_INCLUDE_DIR}
        ${FAASM_WAVM_INCLUDE_DIR} # Required for using WAVM elsewhere
        ${CMAKE_CURRENT_BINARY_DIR}/src # Required for protobuf
)

set(CMAKE_CXX_STANDARD 11)

add_subdirectory(WAVM)
add_subdirectory(func)
add_subdirectory(src/proto)
add_subdirectory(src/util)
add_subdirectory(src/infra)
add_subdirectory(src/wasm)
add_subdirectory(src/worker)
add_subdirectory(src/edge)

add_subdirectory(test)
