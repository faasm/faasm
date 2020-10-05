cmake_minimum_required(VERSION 3.0)
project(dynlink)

set(BUILD_SHARED_LIBS ON)

include_directories(include)

# This is critical otherwise the linker tries to statically link in standard
# libraries
set(CMAKE_C_FLAGS "-fPIC -nostdlib -nostdlib++ ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "-fPIC -nostdlib -nostdlib++ ${CMAKE_CXX_FLAGS}")

if (FAASM_BUILD_TYPE STREQUAL "wasm")
    # See note in toolchain repo on shared libraries only supported by
    set(CMAKE_C_COMPILER_TARGET wasm32-unknown-emscripten)
    set(CMAKE_CXX_COMPILER_TARGET wasm32-unknown-emscripten)
endif ()

add_subdirectory(libA)
add_subdirectory(libB)
