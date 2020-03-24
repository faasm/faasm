cmake_minimum_required(VERSION 3.0)
project(libfaasmpi)

# ------ Faasm MPI library ------ #

set(CMAKE_CXX_STANDARD 17)

set(PROJ_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/../../include/faasmpi)
include_directories(${CMAKE_CURRENT_LIST_DIR} ${PROJ_INCLUDE})

set(PUBLIC_HEADERS
        faasmpi/faasmpi.h
        )

set(LIB_FILES
        mpi.cpp
        )

if (FAASM_BUILD_TYPE STREQUAL "wasm")
    add_library(faasmpi STATIC ${LIB_FILES})
    set_target_properties(faasmpi PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

    # Install in WASM sysroot
    install(TARGETS faasmpi
            ARCHIVE DESTINATION ${CMAKE_SYSROOT}/lib
            LIBRARY DESTINATION ${CMAKE_SYSROOT}/lib
            PUBLIC_HEADER DESTINATION ${CMAKE_SYSROOT}/include/faasmpi
            )

    # Add imports
    install(FILES faasmpi.imports DESTINATION ${CMAKE_SYSROOT}/lib RENAME libfaasmpi.imports)

    # Add mpi header (already present in native builds)
    install(FILES ${PROJ_INCLUDE}/mpi.h DESTINATION ${CMAKE_SYSROOT}/include)
else ()
    add_library(faasmpi SHARED ${LIB_FILES})
    set_target_properties(faasmpi PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

    install(TARGETS faasmpi
            ARCHIVE DESTINATION lib
            LIBRARY DESTINATION lib
            PUBLIC_HEADER DESTINATION include/faasmpi
            )
endif ()