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

set(FAASM_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(FAASM_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)

include_directories(
        ${FAASM_INCLUDE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/src
        ${CMAKE_CURRENT_BINARY_DIR}/WAVM/Source/Include
)

set(CMAKE_CXX_STANDARD 11)

add_subdirectory(WAVM)
add_subdirectory(src/proto)
add_subdirectory(src/util)
add_subdirectory(src/infra)
add_subdirectory(src/worker)
add_subdirectory(src/edge)

add_subdirectory(test)
