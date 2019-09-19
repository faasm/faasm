cmake_minimum_required(VERSION 3.0)
project(libpyfaasm)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_CURRENT_LIST_DIR})

set(LIB_FILES pyfaasm.c)
set(PUBLIC_HEADERS faasm/pyfaasm.h)

if(FAASM_BUILD_TYPE STREQUAL "wasm")
    add_library(pyfaasm STATIC ${LIB_FILES})
    set_target_properties(pyfaasm PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

    add_definitions(-D__WASM__=1)

    install(TARGETS pyfaasm
            ARCHIVE DESTINATION ${CMAKE_SYSROOT}/lib
            LIBRARY DESTINATION ${CMAKE_SYSROOT}/lib
            RUNTIME DESTINATION ${CMAKE_SYSROOT}/bin
            PUBLIC_HEADER DESTINATION ${CMAKE_SYSROOT}/include/faasm
            )
else()
    add_library(pyfaasm SHARED ${LIB_FILES})
    set_target_properties(pyfaasm PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")
endif()
