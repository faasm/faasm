cmake_minimum_required(VERSION 3.0)
project(faasm)

SET(CMAKE_CXX_FLAGS "-g -Wall -Werror")

enable_testing()

set(FAASM_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(FAASM_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)

include_directories(${FAASM_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/src)

set(CMAKE_CXX_STANDARD 11)

add_subdirectory(src/worker)
add_subdirectory(src/edge)
add_subdirectory(src/proto)
add_subdirectory(src/redis)
add_subdirectory(src/util)

add_subdirectory(test)
