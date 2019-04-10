cmake_minimum_required(VERSION 3.0)
project(libfake)

include_directories(include)

set(BUILD_SHARED_LIBS ON)

if (FAASM_BUILD_TYPE STREQUAL "emscripten")
    message(STATUS "Detected emscripten build")

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMPILER_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMPILER_FLAGS}")

    set(CMAKE_SHARED_LINKER_FLAGS " \
        -Xlinker --allow-undefined \
        -Xlinker --no-entry \
        -Xlinker --stack-first \
        -Xlinker --export=mult \
        -Xlinker --import-memory \
        -Xlinker --import-table \
    ")
endif ()

add_library(fake SHARED include/fake.h src/fake.c)
set_target_properties(fake PROPERTIES PUBLIC_HEADER include/fake.h)

install(TARGETS fake
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        PUBLIC_HEADER DESTINATION include
        )
