cmake_minimum_required(VERSION 3.0)
project(libfaasm)

# Include Faasm headers
include_directories(
        ${CMAKE_CURRENT_LIST_DIR}
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpic -fvisibility=default")

# For some incredibly annoying reason, CMake isn't picking this up from the environment
# It's crucial for getting things to link properly so unfortunately must be set here
if (CMAKE_BUILD_TYPE MATCHES "wasm")
    set(CMAKE_RANLIB /toolchain/bin/llvm-ranlib)
endif()

add_library(faasm STATIC
        counter.cpp
        matrix.cpp
        memory.cpp
        random.cpp
        )

set(PUBLIC_HEADERS
        faasm/faasm.h
        faasm/counter.h
        faasm/matrix.h
        faasm/memory.h
        faasm/random.h
        )

set_target_properties(faasm PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

INSTALL(TARGETS faasm
        ARCHIVE DESTINATION /toolchain/sysroot/lib
        LIBRARY DESTINATION /toolchain/sysroot/lib
        PUBLIC_HEADER DESTINATION /toolchain/sysroot/include/faasm
        )

# Copy imports file into place
install(FILES libfaasm.imports DESTINATION /toolchain/sysroot/lib/)
