cmake_minimum_required(VERSION 3.0)
project(libfake)

include_directories(include)

if (FAASM_BUILD_TYPE STREQUAL "wasm")
    # Set up correct compiler flags for building a lib
    set(CMAKE_CXX_FLAGS "${LIB_COMPILER_FLAGS}")
    set(CMAKE_C_FLAGS "${LIB_COMPILER_FLAGS}")
elseif(${EMSCRIPTEN})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")

    # These flags should match what's set for SIDE_LDFLAGS in pyodide
    # https://github.com/iodide-project/pyodide/blob/master/Makefile.envs
    set(CMAKE_SHARED_LINKER_FLAGS "-s \"BINARYEN_METHOD='native-wasm'\" \
        -s \"BINARYEN_TRAP_MODE='clamp'\" \
        -s SIDE_MODULE=1 \
	    -s EXPORT_ALL=1 \
	    -s EXPORTED_FUNCTIONS='[\"_mult\"]' \
	    -s WASM=1 \
        ")

    set(CMAKE_SHARED_LIBRARY_SUFFIX ".wasm")
endif()

set(BUILD_SHARED_LIBS ON)
add_library(fake SHARED include/fake.h src/fake.c)
set_target_properties(fake PROPERTIES PUBLIC_HEADER include/fake.h)

install(TARGETS fake
        ARCHIVE DESTINATION ${WASM_SYSROOT}/lib
        LIBRARY DESTINATION ${WASM_SYSROOT}/lib
        PUBLIC_HEADER DESTINATION ${WASM_SYSROOT}/include
        )
