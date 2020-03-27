cmake_minimum_required(VERSION 3.0)
project(dynlink)

set(BUILD_SHARED_LIBS ON)

include_directories(include)

# This is critical otherwise the linker tries to statically link in standard libraries
set(CMAKE_C_FLAGS "-fPIC -nostdlib ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "-fPIC -nostdlib ${CMAKE_CXX_FLAGS}")

if (FAASM_BUILD_TYPE STREQUAL "wasm")
    # NOTE: According to this LLVM bug: https://bugs.llvm.org/show_bug.cgi?id=42714
    # ONLY the wasm32-unknown-emscripten triple supports fPIC wasm, so we must override
    # whatever is already sets
    # set(CMAKE_C_COMPILER_TARGET wasm32-unknown-emscripten)
    # set(CMAKE_CXX_COMPILER_TARGET wasm32-unknown-emscripten)

    message(STATUS "Detected wasm build")

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
