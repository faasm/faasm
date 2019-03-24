#!/bin/bash

set -e

FAASM_HOME=${HOME}/faasm
SYSROOT=${FAASM_HOME}/sysroot

LLVM_DIR=${FAASM_HOME}/llvm-project
LIBCXX_ABI_INCLUDE=${LLVM_DIR}/libcxxabi/include
LIBCXX_INCLUDE=${SYSROOT}/include/c++/v1

LIBC_MODULE=${SYSROOT}/lib/libc.a
COMPILER_FLAGS="--target=wasm32-unknown-unknown -nostdlib -nostdlib++ --sysroot=${SYSROOT} ${LIBC_MODULE} -Xlinker --no-entry -Xlinker --import-memory"

if [ -d ${LLVM_DIR} ]; then
   echo "Not cloning llvm, already exists"
else
    echo "Cloning llvm repo"
    pushd ${FAASM_HOME}
    git clone https://github.com/llvm/llvm-project.git
    popd
fi

rm -rf ${LLVM_DIR}/build
mkdir -p ${LLVM_DIR}/build
pushd ${LLVM_DIR}/build

# DLIBCXX_HERMETIC_STATIC_LIBRARY=ON \
# DLIBCXXABI_HERMETIC_STATIC_LIBRARY=ON \

VERBOSE=1 cmake \
  -DCMAKE_C_FLAGS="${COMPILER_FLAGS}" \
  -DCMAKE_CXX_FLAGS="${COMPILER_FLAGS}" \
  -DCMAKE_C_COMPILER="/usr/bin/clang-8" \
  -DCMAKE_CXX_COMPILER="/usr/bin/clang++-8" \
  -DLLVM_ENABLE_PROJECTS="libcxx;libcxxabi" \
  -DLLVM_COMPILER_CHECKED=ON \
  -DLIBCXX_CXX_ABI_INCLUDE_PATHS=${LIBCXX_ABI_INCLUDE} \
  -DLIBCXXABI_LIBCXX_PATH=${LLVM_DIR}/libcxx \
  -DLIBCXXABI_LIBCXX_INCLUDES=${LIBCXX_INCLUDE} \
  -DCMAKE_BUILD_TYPE=Release \
  -DLIBCXX_HAS_MUSL_LIBC=ON \
  -DLIBCXX_ENABLE_THREADS=OFF \
  -DLIBCXX_ENABLE_SHARED=OFF \
  -DLIBCXXABI_ENABLE_SHARED=OFF \
  -DLIBCXXABI_ENABLE_THREADS=OFF \
  -DLIBCXX_ENABLE_ASSERTIONS=OFF \
  -DLIBCXX_CXX_ABI=libcxxabi \
  ../llvm

popd
