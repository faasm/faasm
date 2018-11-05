cmake_minimum_required(VERSION 3.0)
project(libfaasm)

## Include Faasm headers
#include_directories(
#        ${CMAKE_CURRENT_LIST_DIR}
#)

set(PUBLIC_HEADERS
        faasm/faasm.h
        faasm/counter.h
        faasm/matrix.h
        faasm/random.h
        )

add_library(faasm STATIC
        counter.cpp
        matrix.cpp
        random.cpp
        faasm.cpp
        ${PUBLIC_HEADERS}
)

set_target_properties(faasm PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

INSTALL(TARGETS faasm
        ARCHIVE DESTINATION /toolchain/sysroot/lib
        LIBRARY DESTINATION /toolchain/sysroot/lib
        PUBLIC_HEADER DESTINATION /toolchain/sysroot/include/faasm
)

# Copy imports file into place
install(FILES libfaasm.imports DESTINATION /toolchain/sysroot/lib/)
