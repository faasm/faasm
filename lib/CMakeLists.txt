cmake_minimum_required(VERSION 3.0)
project(libfaasm)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_CURRENT_LIST_DIR})

set(LIB_FILES
        counter.cpp
        func.cpp
        matrix.cpp
        memory.cpp
        print.cpp
        random.cpp
        sgd.cpp
        time.cpp
        )

set(PUBLIC_HEADERS
        faasm/counter.h
        faasm/faasm.h
        faasm/func.h
        faasm/matrix.h
        faasm/memory.h
        faasm/print.h
        faasm/random.h
        faasm/sgd.h
        faasm/time.h
        )

if (FAASM_BUILD_TYPE STREQUAL "emscripten")
    add_definitions(-DWASM_BUILD=1)
    set(MALLOC_LIB "dlmalloc")
else ()
    add_definitions(-DWASM_BUILD=0)
    set(MALLOC_LIB "")
endif ()

add_library(faasm STATIC ${LIB_FILES})

target_link_libraries(faasm ${MALLOC_LIB})

set_target_properties(faasm PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

install(TARGETS faasm
        ARCHIVE DESTINATION ${CMAKE_SYSROOT}/lib
        LIBRARY DESTINATION ${CMAKE_SYSROOT}/lib
        RUNTIME DESTINATION ${CMAKE_SYSROOT}/bin
        PUBLIC_HEADER DESTINATION ${CMAKE_SYSROOT}/include/faasm
        )

install(FILES libfaasm.imports DESTINATION ${CMAKE_SYSROOT}/lib/)
