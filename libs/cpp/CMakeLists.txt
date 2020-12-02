cmake_minimum_required(VERSION 3.0)
project(libfaasm)

# ------ Faasm C/C++ library ------ #

set(CMAKE_CXX_STANDARD 17)

set(PROJ_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/../../include)
include_directories(${CMAKE_CURRENT_LIST_DIR} ${PROJ_INCLUDE})

set(PUBLIC_HEADERS
        ${PROJ_INCLUDE}/faasm/core.h
        ${PROJ_INCLUDE}/faasm/host_interface.h
        faasm/array.h
        faasm/compare.h
        faasm/counter.h
        faasm/faasm.h
        faasm/files.h
        faasm/input.h
        faasm/matrix.h
        faasm/print.h
        faasm/random.h
        faasm/sgd.h
        faasm/state.h
        faasm/time.h
        )

set(LIB_FILES
        compare.cpp
        core.cpp
        counter.cpp
        files.cpp
        input.cpp
        matrix.cpp
        print.cpp
        random.cpp
        sgd.cpp
        state.cpp
        time.cpp
        zygote.cpp
        )


if (CMAKE_SYSTEM_NAME STREQUAL "Wasm")
    add_library(faasm STATIC ${LIB_FILES})
    set_target_properties(faasm PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

    # Install in WASM sysroot
    install(TARGETS faasm
            ARCHIVE DESTINATION ${CMAKE_SYSROOT}/lib/wasm32-wasi
            LIBRARY DESTINATION ${CMAKE_SYSROOT}/lib/wasm32-wasi
            PUBLIC_HEADER DESTINATION ${CMAKE_SYSROOT}/include/faasm
            )

    # Add allowed import files
    install(
            FILES ${PROJ_INCLUDE}/faasm/faasm.imports
            DESTINATION ${CMAKE_SYSROOT}/lib/wasm32-wasi
            RENAME libfaasm.imports
    )
else ()
    # Note - to properly link with native code, this must be built
    # with FAASM_STATIC_LIBS=OFF
    faasm_public_lib(faasm "${LIB_FILES}")

    add_dependencies(faasm eigen_ext)
    set_target_properties(faasm PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

    install(TARGETS faasm
            ARCHIVE DESTINATION lib
            LIBRARY DESTINATION lib
            PUBLIC_HEADER DESTINATION include/faasm
            )
endif ()
