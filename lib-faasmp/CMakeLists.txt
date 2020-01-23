cmake_minimum_required(VERSION 3.0)
project(libfaasmp)

# ------ Faasm OpenMP library ------ #

set(CMAKE_CXX_STANDARD 17)

set(PUBLIC_HEADERS
        faasmp/faasmp.h
        )

set(LIB_FILES
        faasmp.cpp
        )

if (FAASM_BUILD_TYPE STREQUAL "wasm")
    add_library(faasmp STATIC ${LIB_FILES})
    set_target_properties(faasmp PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

    # Install in WASM sysroot
    install(TARGETS faasmp
            ARCHIVE DESTINATION ${CMAKE_SYSROOT}/lib
            LIBRARY DESTINATION ${CMAKE_SYSROOT}/lib
            PUBLIC_HEADER DESTINATION ${CMAKE_SYSROOT}/include/faasmp
            )

    # Add imports
    install(FILES faasmp.imports DESTINATION ${CMAKE_SYSROOT}/lib RENAME libfaasmp.imports)

    # Add omp and mpi headers (already present in native builds)
    install(FILES omp.h DESTINATION ${CMAKE_SYSROOT}/include)
    install(FILES mpi.h DESTINATION ${CMAKE_SYSROOT}/include)
    install(FILES mpi_portable_platform.h DESTINATION ${CMAKE_SYSROOT}/include)
else ()
    add_library(faasmp SHARED ${LIB_FILES})
    set_target_properties(faasmp PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

    install(TARGETS faasmp
            ARCHIVE DESTINATION lib
            LIBRARY DESTINATION lib
            PUBLIC_HEADER DESTINATION include/faasmp
            )
endif ()