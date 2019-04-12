cmake_minimum_required(VERSION 3.0)
project(libfake)

# Need to use something from the importing environment, hence can include libfaasm headers
include_directories(include faasm/faasm.h)

set(BUILD_SHARED_LIBS ON)

# This is critical otherwise the linker tries to statically link in standard libraries
set(COMPILER_FLAGS "-nostdlib -nostdlib++")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMPILER_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMPILER_FLAGS}")

if (FAASM_BUILD_TYPE STREQUAL "emscripten")
    message(STATUS "Detected emscripten build")

    # NOTE: --stack-first and --shared are crucial here too to getting the linker to produce a properly
    # formatted shared object
    set(CMAKE_SHARED_LINKER_FLAGS " \
        -Xlinker --no-entry \
        -Xlinker --shared \
        -Xlinker --stack-first \
        -Xlinker --export-all \
    ")

endif ()

add_library(fake SHARED include/fake.h src/fake.c)
set_target_properties(fake PROPERTIES PUBLIC_HEADER include/fake.h)

install(TARGETS fake
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        PUBLIC_HEADER DESTINATION include
        )
