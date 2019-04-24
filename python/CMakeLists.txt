cmake_minimum_required(VERSION 3.0)
project(libpyfaasm)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_CURRENT_LIST_DIR})

set(LIB_FILES pyfaasm.c)
set(PUBLIC_HEADERS faasm/pyfaasm.h)

add_library(pyfaasm STATIC ${LIB_FILES})
set_target_properties(pyfaasm PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

install(TARGETS pyfaasm
        ARCHIVE DESTINATION ${CMAKE_SYSROOT}/lib
        LIBRARY DESTINATION ${CMAKE_SYSROOT}/lib
        RUNTIME DESTINATION ${CMAKE_SYSROOT}/bin
        PUBLIC_HEADER DESTINATION ${CMAKE_SYSROOT}/include/faasm
        )
