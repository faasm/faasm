cmake_minimum_required(VERSION 3.0)
project(faasmpi)

# ------------------------------------------
# This builds the Faabric MPI library to wasm and 
# adds the relevant .imports file
# ------------------------------------------

set(CMAKE_CXX_STANDARD 17)

include (ExternalProject)
include (FetchContent)

set(PUBLIC_HEADERS
    faasmpi/faasmpi.h
)

set(LIB_FILES
    faasmpi.cpp
)

add_library(faasmpi STATIC ${LIB_FILES})
set_target_properties(faasmpi PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

# Only do the install if we're doing a WASM build
# WARNING - this overwrites the existing mpi.h on the system if not used in a
# wasm build
if (FAASM_BUILD_TYPE STREQUAL "wasm")    
    # We need to build just the Faabric MPI library to wasm here
    # TODO - split Faabric MPI library out into a separate repo 
    FetchContent_Declare(faabric_ext
        GIT_REPOSITORY "https://github.com/faasm/faabric.git"
        GIT_TAG "v0.0.13"
        UPDATE_COMMAND ""
        CMAKE_ARGS "-DFAABRIC_BUILD_TESTS=OFF"
    )

    FetchContent_Populate(faabric_ext)
    FetchContent_GetProperties(faabric_ext SOURCE_DIR FAABRIC_SOURCE_DIR)
    FetchContent_GetProperties(faabric_ext BINARY_DIR FAABRIC_BINARY_DIR)
    add_subdirectory(${FAABRIC_SOURCE_DIR}/src/mpi ${FAABRIC_BINARY_DIR})
    add_dependencies(faasmpi faabricmpi)

    # Install in sysroot
    install(TARGETS faasmpi
        ARCHIVE DESTINATION ${CMAKE_SYSROOT}/lib/wasm32-wasi
        LIBRARY DESTINATION ${CMAKE_SYSROOT}/lib/wasm32-wasi
        PUBLIC_HEADER DESTINATION ${CMAKE_SYSROOT}/include/faasmpi
    )

    # Add imports
    install(FILES faasmpi.imports DESTINATION 
        ${CMAKE_SYSROOT}/lib/wasm32-wasi RENAME libfaasmpi.imports
    )

    # Symlink header
    install(CODE "execute_process( \
        COMMAND ${CMAKE_COMMAND} -E create_symlink \
        ${CMAKE_SYSROOT}/include/faabric/mpi/mpi.h \
        ${CMAKE_SYSROOT}/include/mpi.h   \
        )"
    )

    # Symlink library
    install(CODE "execute_process( \
        COMMAND ${CMAKE_COMMAND} -E create_symlink \
        ${CMAKE_SYSROOT}/lib/wasm32-wasi/libfaabricmpi.a \
        ${CMAKE_SYSROOT}/lib/wasm32-wasi/libmpi.a   \
        )"
    )

endif()
