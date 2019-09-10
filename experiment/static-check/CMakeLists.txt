cmake_minimum_required(VERSION 3.0)
project(static-test)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_CURRENT_LIST_DIR})

set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} -Wl,-rpath='$ORIGIN'")

add_library(check STATIC dummy.cpp)
target_link_libraries(check PUBLIC dlmalloc png)
