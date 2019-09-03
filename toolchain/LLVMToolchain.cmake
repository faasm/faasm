cmake_minimum_required(VERSION 3.4.0)

set(EXE_SUFFIX "")

set(CMAKE_C_COMPILER /usr/bin/clang-8)
set(CMAKE_CXX_COMPILER /usr/bin/clang++-8)
set(CMAKE_AR /usr/bin/llvm-ar-8)
set(CMAKE_RANLIB /usr/bin/llvm-ranlib-8)
set(CMAKE_LD /usr/bin/wasm-ld-8)

#set(CMAKE_SYSTEM_NAME Generic)
#set(CMAKE_SYSTEM_VERSION 1)
#set(CMAKE_SYSTEM_PROCESSOR wasm32)

set(triple wasm32-unknown-unknown)

set(CMAKE_C_COMPILER_TARGET ${triple})
set(CMAKE_CXX_COMPILER_TARGET ${triple})

#set(CMAKE_INSTALL_PREFIX /usr/local/faasm/llvm-sysroot)

set(CMAKE_SYSROOT /usr/local/faasm/llvm-sysroot)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
