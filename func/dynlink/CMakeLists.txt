cmake_minimum_required(VERSION 3.0)
project(dynlink)

set(BUILD_SHARED_LIBS ON)

include_directories(include)

# This is critical otherwise the linker tries to statically link in standard libraries
set(COMPILER_FLAGS "-nostdlib -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMPILER_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMPILER_FLAGS}")

if (FAASM_BUILD_TYPE STREQUAL "emscripten")
    message(STATUS "Detected emscripten build")

    # NOTE: all of these linker options are crucial to getting things to work properly
    set(CMAKE_SHARED_LINKER_FLAGS " \
        -Xlinker --shared \
        -Xlinker --stack-first \
        -Wl,-z,stack-size=2097152 -Wl, \
        -Xlinker --no-entry \
        -Xlinker --export-all \
        -Xlinker --export-dynamic \
        -Xlinker --no-gc-sections \
    ")
endif ()

add_subdirectory(libA)
add_subdirectory(libB)