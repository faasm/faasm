cmake_minimum_required(VERSION 3.0)
project(pyinit)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_CURRENT_LIST_DIR})

set(LIB_FILES pyinit.c)
set(PUBLIC_HEADERS faasm/pyinit.h)

if(FAASM_BUILD_TYPE STREQUAL "wasm")
    add_library(pyinit STATIC ${LIB_FILES})
    set_target_properties(pyinit PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

    install(TARGETS pyinit
            ARCHIVE DESTINATION ${CMAKE_SYSROOT}/lib
            LIBRARY DESTINATION ${CMAKE_SYSROOT}/lib
            RUNTIME DESTINATION ${CMAKE_SYSROOT}/bin
            PUBLIC_HEADER DESTINATION ${CMAKE_SYSROOT}/include/faasm
            )
else()
    add_library(pyinit SHARED ${LIB_FILES})
    set_target_properties(pyinit PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")
endif()
