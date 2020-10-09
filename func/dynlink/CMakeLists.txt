cmake_minimum_required(VERSION 3.0)
project(dynlink)

set(BUILD_SHARED_LIBS ON)

include_directories(include)

add_subdirectory(libA)
add_subdirectory(libB)
