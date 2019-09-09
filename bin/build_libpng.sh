#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
PROJ_ROOT=${THIS_DIR}/..
source ${PROJ_ROOT}/toolchain/env.sh

pushd ${PROJ_ROOT}/libpng

CC=${WASM_CC} \
CXX=${WASM_CXX} \
CPP=${WASM_CPP} \
AR=${WASM_AR} \
RANLIB=${WASM_RANLIB} \
CFLAGS="${WASM_CFLAGS}" \
CXXFLAGS="${WASM_CFLAGS}" \
CPPFLAGS="${WASM_CPPFLAGS}" \
LDFLAGS=${WASM_LDFLAGS} \
./configure \
  --build=${WASM_BUILD} \
  --host=${WASM_HOST} \
  --prefix=${WASM_SYSROOT} ; 

make

make install

popd
