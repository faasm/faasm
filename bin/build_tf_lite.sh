#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
PROJ_ROOT=${THIS_DIR}/..
TF_DIR=${PROJ_ROOT}/third-party/tensorflow
TF_LITE_DIR=${TF_DIR}/tensorflow/lite

FAASM_TOOLCHAIN=${PROJ_ROOT}/toolchain
source ${FAASM_TOOLCHAIN}/env.sh

pushd ${TF_DIR}

# Tensorflow will try to include its own stuff which we want to override
INCLUDES="-I. -I${TF_DIR}"
LIBS=-lstdc++
LDOPTS=""

make -j 4 \
    CC=${WASM_CC} \
    CXX=${WASM_CXX} \
    CPP=${WASM_CPP} \
    AR=${WASM_AR} \
    RANLIB=${WASM_RANLIB} \
    CFLAGS="${WASM_CFLAGS} -ftls-model=local-exec -ldlmalloc" \
    CXXFLAGS="${CFLAGS}" \
    CPPFLAGS="${WASM_CPPFLAGS}" \
    LDFLAGS="${WASM_LDFLAGS} -Xlinker --no-check-features -Xlinker --max-memory=1073741824" \
    TARGET=${WASM_HOST} \
    TARGET_ARCH=${WASM_TARGET} \
    INCLUDES=${INCLUDES} \
    LIBS=${LIBS} \
    LDOPTS=${LDOPTS} \
    ARFLAGS= \
    BUILD_TYPE=micro \
    BUILD_WITH_NNAPI=false \
    BENCHMARK_LIB_SRCS= \
    CMD_LINE_TOOLS_SRCS= \
    PROFILE_SUMMARIZER_SRCS= \
    PROFILER_SRCS= \
    -C "${TF_DIR}" \
    -f tensorflow/lite/tools/make/Makefile \
    minimal

popd
