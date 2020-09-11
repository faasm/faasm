cmake_minimum_required(VERSION 3.0)
project(faasmpi)

# ------------------------------------------
# This is a shell to create a fake MPI-like
# library that just translates to wasm imports
# ------------------------------------------

set(CMAKE_CXX_STANDARD 17)

set(PUBLIC_HEADERS
        faasmpi/faasmpi.h
        )

set(LIB_FILES
        faasmpi.cpp
        )

add_library(faasmpi STATIC ${LIB_FILES})
set_target_properties(faasmpi PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

# Only do the install if we're doing a WASM build
# WARNING - this overwrites the existing mpi.h on the system if used incorrectly
if (FAASM_BUILD_TYPE STREQUAL "wasm")    
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
