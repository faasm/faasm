cmake_minimum_required(VERSION 3.0)
project(libfaasm)

set(CMAKE_CXX_STANDARD 17)

# Include Faasm headers
include_directories(${CMAKE_CURRENT_LIST_DIR})

# Emscripten doesn't seem to look in normal system includes so we explicitly have to tell it to
if (${EMSCRIPTEN})
    include_directories(/usr/local/include/)
endif ()

set(LIB_FILES
        counter.cpp
        func.cpp
        matrix.cpp
        memory.cpp
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
        faasm/random.h
        faasm/sgd.h
        faasm/time.h
        )

if (FAASM_BUILD_TYPE STREQUAL "wasm")
    set(CMAKE_C_FLAGS "${WASM_COMPILER_FLAGS}")
    set(CMAKE_CXX_FLAGS "${WASM_COMPILER_FLAGS}")

    message(STATUS "Running libfaasm wasm build")

elseif (FAASM_BUILD_TYPE STREQUAL "python")
    add_definitions(-DWASM_BUILD=1)

    set(CMAKE_SHARED_LINKER_FLAGS "-O2 \
        -s EMULATED_FUNCTION_POINTERS=1 \
        -s EMULATE_FUNCTION_POINTER_CASTS=1 \
        -s SIDE_MODULE=1 \
        -s WASM=1 \
        -s LINKABLE=1 \
        -s EXPORT_ALL=1 \
        --memory-init-file 0")

elseif (${EMSCRIPTEN})
    add_definitions(-DWASM_BUILD=1)

    set(CMAKE_SHARED_LINKER_FLAGS "-O2 \
        -s SIDE_MODULE=1 \
        -s WASM=1 \
        -s LINKABLE=1 \
        -s EXPORT_ALL=1 \
        --memory-init-file 0")
else ()
    add_definitions(-DWASM_BUILD=0)
endif ()

add_library(faasm STATIC ${LIB_FILES})

set_target_properties(faasm PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

install(TARGETS faasm
        ARCHIVE DESTINATION ${CMAKE_BINARY_DIR}/lib
        LIBRARY DESTINATION ${CMAKE_BINARY_DIR}/lib
        RUNTIME DESTINATION ${CMAKE_BINARY_DIR}/bin
        PUBLIC_HEADER DESTINATION ${CMAKE_BINARY_DIR}/include
        )

install(FILES libfaasm.imports DESTINATION ${CMAKE_BINARY_DIR}/lib/)
