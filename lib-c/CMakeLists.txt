cmake_minimum_required(VERSION 3.0)
project(libfaasmc)

# ------ Faasm C library ------ #

set(PROJ_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/../include)

include_directories(${PROJ_INCLUDE})

set(PUBLIC_HEADERS
        ${PROJ_INCLUDE}/faasm/core.h
        faasmc/faasm.h
        )

set(LIB_FILES
        core.c
        )

if (FAASM_BUILD_TYPE STREQUAL "wasm")
    add_definitions(-DWASM_BUILD=1)
    set(MALLOC_LIB "dlmalloc")
else ()
    add_definitions(-DWASM_BUILD=0)
    set(MALLOC_LIB "")
endif ()

add_library(faasmc STATIC ${LIB_FILES} ${PUBLIC_HEADERS})

target_link_libraries(faasmc ${MALLOC_LIB})

set_target_properties(faasmc PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

install(TARGETS faasmc
        ARCHIVE DESTINATION ${CMAKE_SYSROOT}/lib
        LIBRARY DESTINATION ${CMAKE_SYSROOT}/lib
        RUNTIME DESTINATION ${CMAKE_SYSROOT}/bin
        PUBLIC_HEADER DESTINATION ${CMAKE_SYSROOT}/include/faasmc
        )

install(
        FILES ${PROJ_INCLUDE}/faasm/faasm.imports
        DESTINATION ${CMAKE_SYSROOT}/lib
        RENAME libfaasmc.imports
)
