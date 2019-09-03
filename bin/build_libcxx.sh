#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
PROJ_ROOT=${THIS_DIR}/..
LLVM_DIR=${PROJ_ROOT}/llvm-project

WASM_TOOLCHAIN=${PROJ_ROOT}/LLVMToolchain.cmake
INSTALL_DIR=/usr/local/faasm/llvm-sysroot

pushd ${LLVM_DIR}

mkdir -p build

pushd build

mkdir -p ${INSTALL_DIR}

# Build with wasm toolchain
cmake \
    -G Ninja \
    -DLLVM_ENABLE_PROJECTS="libcxx;libcxxabi;compiler-rt;libunwind" \
    -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} \
    -DCMAKE_TOOLCHAIN_FILE=${WASM_TOOLCHAIN} \
    -DCMAKE_CXX_COMPILER_WORKS:BOOL=ON \
    -DCMAKE_C_COMPILER_WORKS:BOOL=ON \
    -DLIBCXX_INCLUDE_TESTS:BOOL=NO \
    -DLIBCXX_USE_COMPILER_RT:BOOL=OFF \
    -DLIBCXXABI_USE_COMPILER_RT:BOOL=ON \
    -DLIBCXXABI_ENABLE_EXCEPTIONS:BOOL=OFF \
    -DLIBCXXABI_ENABLE_SHARED:BOOL=OFF \
    -DLIBCXXABI_SILENT_TERMINATE:BOOL=ON \
    -DLIBCXXABI_ENABLE_THREADS:BOOL=ON \
    -DLIBCXXABI_HAS_PTHREAD_API:BOOL=ON \
    -DLIBCXXABI_HAS_EXTERNAL_THREAD_API:BOOL=OFF \
    -DLIBCXXABI_BUILD_EXTERNAL_THREAD_LIBRARY:BOOL=OFF \
    -DLIBCXXABI_HAS_WIN32_THREAD_API:BOOL=OFF \
    ../llvm

popd

popd
