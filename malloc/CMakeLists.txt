cmake_minimum_required(VERSION 3.0)
project(faasm-malloc)

include_directories(${CMAKE_CURRENT_LIST_DIR})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

add_library(dlmalloc STATIC dlmalloc.c)

set_target_properties(dlmalloc PROPERTIES PUBLIC_HEADER malloc.h)

install(TARGETS dlmalloc
        ARCHIVE DESTINATION ${CMAKE_SYSROOT}/lib
        LIBRARY DESTINATION ${CMAKE_SYSROOT}/lib
        RUNTIME DESTINATION ${CMAKE_SYSROOT}/bin
        PUBLIC_HEADER DESTINATION ${CMAKE_SYSROOT}/include/
        )

