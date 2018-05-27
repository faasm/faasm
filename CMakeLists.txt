cmake_minimum_required(VERSION 3.5)
project(faasm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_COMPILER /usr/bin/clang++)
set(CMAKE_C_COMPILER /usr/bin/clang)

#set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_VERBOSE_MAKEFILE ON)

add_executable(main main.cpp)

set(V8_DIR /usr/local/code/v8)
include_directories(${V8_DIR} ${V8_DIR}/include  ${V8_BUILD_DIR})

set(V8_BUILD_DIR ${V8_DIR}/out.gn/x64.release)

find_library(V8_LIB v8 HINTS ${V8_BUILD_DIR})
find_library(V8_LIBBASE v8_libbase HINTS ${V8_BUILD_DIR})
find_library(V8_LIBPLATFORM v8_libplatform HINTS ${V8_BUILD_DIR})

target_include_directories(main PRIVATE ${V8_DIR} ${V8_DIR}/include ${V8_BUILD_DIR})
target_link_libraries(main LINK_PUBLIC -stdlib=libc++ ${V8_LIB} ${V8_LIBBASE} ${V8_LIBPLATFORM} -lrt -ldl -lpthread )

# Note, this should be all the binaries created in the base of out.gn/x64.release
set(V8_FILES
        ${V8_BUILD_DIR}/natives_blob.bin
        ${V8_BUILD_DIR}/snapshot_blob.bin
)

foreach(f ${V8_FILES})
    file(COPY ${f} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endforeach(f)