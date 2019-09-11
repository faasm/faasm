# NOTE - this is the legacy emscripten upstream toolchain which should
# no longer be needed.

cmake_minimum_required(VERSION 3.4.0)

set(EMSDK_ROOT /usr/local/faasm/emsdk/upstream/latest)
set(EMSDK_SYSROOT ${EMSDK_ROOT}/sysroot)

set(EXE_SUFFIX "")

set(CMAKE_C_COMPILER ${EMSDK_ROOT}/bin/wasm32-clang${EXE_SUFFIX})
set(CMAKE_CXX_COMPILER ${EMSDK_ROOT}/bin/wasm32-clang++${EXE_SUFFIX})
set(CMAKE_AR ${EMSDK_ROOT}/bin/llvm-ar${EXE_SUFFIX} CACHE FILEPATH "llvm ar")
set(CMAKE_RANLIB ${EMSDK_ROOT}/bin/llvm-ranlib${EXE_SUFFIX} CACHE FILEPATH "llvm ranlib")

set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR wasm32)
set(triple wasm32-unknown-unknown-wasm)

set(UNIX 1)

set(CMAKE_C_COMPILER_TARGET ${triple})
set(CMAKE_CXX_COMPILER_TARGET ${triple})

set(CMAKE_SYSROOT ${EMSDK_SYSROOT})
set(CMAKE_STAGING_PREFIX ${EMSDK_SYSROOT})

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
