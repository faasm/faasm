#!/bin/bash

set -e

# NOTE - requires onnxruntime protobuf to be cloned

THIS_DIR=$(dirname $(readlink -f $0))
PROJ_ROOT=${THIS_DIR}/..
PROTOBUF_DIR=${PROJ_ROOT}/onnxruntime/cmake/external/protobuf

FAASM_TOOLCHAIN=${PROJ_ROOT}/toolchain
source ${FAASM_TOOLCHAIN}/env.sh

LOCAL_PROTOC=/usr/bin/protoc

pushd ${PROTOBUF_DIR}

if [[ ! -f configure ]]; then
    ./autogen.sh
fi

CC=${WASM_CC} \
CXX=${WASM_CXX} \
CPP=${WASM_CPP} \
AR=${WASM_AR} \
RANLIB=${WASM_RANLIB} \
CFLAGS="${WASM_CFLAGS} -ftls-model=local-exec -ldlmalloc" \
CXXFLAGS="${CFLAGS}" \
CPPFLAGS="${WASM_CPPFLAGS}" \
LDFLAGS="${WASM_LDFLAGS} -Xlinker --no-check-features -Xlinker --max-memory=1073741824" \
./configure \
     --disable-threads \
     --disable-shared \
     --disable-tests \
     --with-protoc=${LOCAL_PROTOC} \
     --prefix=${WASM_SYSROOT} \
     --host=${WASM_HOST} \
     --build=${WASM_BUILD}

make
make install

popd
