cmake_minimum_required(VERSION 3.0)
project(libfake)

include_directories(include)

if(${EMSCRIPTEN})
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
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=default")
endif()

add_library(fake SHARED src/fake.c)

set_target_properties(fake PROPERTIES PUBLIC_HEADER include/fake.h)

install(TARGETS fake
        ARCHIVE DESTINATION ${CMAKE_BINARY_DIR}/lib
        LIBRARY DESTINATION /usr/local/faasm/runtime_root
        RUNTIME DESTINATION ${CMAKE_BINARY_DIR}/bin
        PUBLIC_HEADER DESTINATION ${CMAKE_BINARY_DIR}/include
        )
