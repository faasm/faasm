#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
PROJ_ROOT=${THIS_DIR}/..
TF_DIR=${PROJ_ROOT}/third-party/tensorflow
TF_LITE_DIR=${TF_DIR}/tensorflow/lite

FAASM_TOOLCHAIN=${PROJ_ROOT}/toolchain
source ${FAASM_TOOLCHAIN}/env.sh

FAASM_SYSROOT=/usr/local/faasm/llvm-sysroot

pushd ${TF_DIR}

# Tensorflow will try to include its own stuff which we want to override
# Don't split this into multiple lines as it'll break the Makefile
INCLUDES="-I${TF_DIR} -I${TF_LITE_DIR} -I${FAASM_SYSROOT}/include/gemmlowp -I${FAASM_SYSROOT}/include/eigen3"

LIBS=-lstdc++
LDOPTS=""

make -j 4 \
    VERBOSE=1 \
    CC=${WASM_CC} \
    CXX=${WASM_CXX} \
    CPP=${WASM_CPP} \
    AR=${WASM_AR} \
    RANLIB=${WASM_RANLIB} \
    CFLAGS="${WASM_CFLAGS} -ftls-model=local-exec -ldlmalloc" \
    CXXFLAGS="${WASM_CXXFLAGS}" \
    CPPFLAGS="${WASM_CPPFLAGS}" \
    LDFLAGS="${WASM_LDFLAGS} -Xlinker --no-check-features -Xlinker --max-memory=1073741824" \
    TARGET=${WASM_HOST} \
    TARGET_ARCH=${WASM_TARGET} \
    INCLUDES="${INCLUDES}" \
    LIBS=${LIBS} \
    LDOPTS=${LDOPTS} \
    ARFLAGS= \
    BUILD_WITH_NNAPI=OFF \
    -C "${TF_DIR}" \
    -f tensorflow/lite/tools/make/Makefile

popd
