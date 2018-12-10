cmake_minimum_required(VERSION 3.0)
project(faasm)

set(CMAKE_CXX_FLAGS "-Wall -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "-g")

# Address sanitizer
# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -O0")

# set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Oz")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

enable_testing()

#set(WAVM_ENABLE_ASAN OFF)
#set(WAVM_ENABLE_UBSAN OFF)
#set(WAVM_ENABLE_TSAN OFF)

# Stops WAVM compilation complaining
add_definitions(-DDLL_EXPORT=)
add_definitions(-DDLL_IMPORT=)

set(FAASM_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(FAASM_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
set(FAASM_LIB_DIR ${CMAKE_CURRENT_LIST_DIR}/lib)
set(FAASM_WAVM_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/WAVM/Lib)
set(FAASM_WAVM_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/WAVM/Include)

include_directories(
        ${FAASM_INCLUDE_DIR}
        ${FAASM_LIB_DIR}
        ${FAASM_WAVM_INCLUDE_DIR} # Required for using WAVM elsewhere
        ${CMAKE_CURRENT_BINARY_DIR}/src # Required for protobuf
)

set(CMAKE_CXX_STANDARD 11)

# WAVM
add_subdirectory(WAVM)

# Faasm functions
add_subdirectory(func)

# Faasm library
add_subdirectory(lib)

# Faasm runtime
add_subdirectory(src/data)
add_subdirectory(src/edge)
add_subdirectory(src/emulator)
add_subdirectory(src/infra)
add_subdirectory(src/prof)
add_subdirectory(src/proto)
add_subdirectory(src/upload)
add_subdirectory(src/util)
add_subdirectory(src/wasm)
add_subdirectory(src/worker)

add_subdirectory(test)
