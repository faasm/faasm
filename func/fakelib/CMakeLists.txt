cmake_minimum_required(VERSION 3.0)
project(libfake)

include_directories(include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpic -fvisibility=default")

set(CMAKE_SHARED_LINKER_FLAGS "-O2 \
    -s SIDE_MODULE=1 \
    -s WASM=1 \
    -s EXPORTED_FUNCTIONS='[\"doubleInt\"]'")

set(CMAKE_SHARED_LIBRARY_SUFFIX ".wasm")
add_library(fake SHARED src/fake.c)

set_target_properties(fake PROPERTIES PUBLIC_HEADER include/fake.h)

install(TARGETS fake
        ARCHIVE DESTINATION ${CMAKE_BINARY_DIR}/lib
        LIBRARY DESTINATION /usr/local/faasm/runtime_root
        RUNTIME DESTINATION ${CMAKE_BINARY_DIR}/bin
        PUBLIC_HEADER DESTINATION ${CMAKE_BINARY_DIR}/include
        )
