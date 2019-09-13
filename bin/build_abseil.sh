#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
PROJ_ROOT=${THIS_DIR}/..
BUILD_DIR=${PROJ_ROOT}/third-party/abseil-cpp/build

FAASM_SYSROOT=/usr/local/faasm/llvm-sysroot
FAASM_TOOLCHAIN=${PROJ_ROOT}/toolchain
TOOLCHAIN_FILE=${FAASM_TOOLCHAIN}/FaasmToolchain.cmake

mkdir -p ${BUILD_DIR}
pushd ${BUILD_DIR}

cmake \
    -DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN_FILE} \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=${FAASM_SYSROOT} \
    ..

make
make install

popd
