#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
PROJ_ROOT=${THIS_DIR}/..
TOOLCHAIN_FILE=${PROJ_ROOT}/toolchain/FaasmToolchain.cmake
FAASM_SYSROOT=/usr/local/faasm/llvm-sysroot

pushd ${PROJ_ROOT}/third-party/flatbuffers

mkdir -p build
pushd build

cmake -DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN_FILE} \
      -DCMAKE_CXX_FLAGS="-ldlmalloc" \
      -DCMAKE_BUILD_TYPE=RelWithDebInfo \
      -DCMAKE_INSTALL_PREFIX=${FAASM_SYSROOT} \
      -DFLATBUFFERS_BUILD_TESTS=OFF \
      --parallel \
      ..

make
make install

popd
popd
