#!/bin/bash

set -e

if [[ -z "$1" ]]; then
  echo "Defaulting to wasm build"
  BUILD_TYPE=wasm
elif [[ "$1" == "native" ]]; then
  echo "Running native build"
  BUILD_TYPE=native
else
  echo "Unrecognised build type $1"
  return 1
fi

THIS_DIR=$(dirname $(readlink -f $0))
PROJ_ROOT=${THIS_DIR}/..
TF_DIR=${PROJ_ROOT}/third-party/tensorflow
TF_LITE_DIR=${TF_DIR}/tensorflow/lite

FAASM_TOOLCHAIN=${PROJ_ROOT}/toolchain
FAASM_SYSROOT=/usr/local/faasm/llvm-sysroot

# Download deps
if [[ ! -d "${TF_LITE_DIR}/tools/make/downloads/absl" ]]; then
    ${TF_LITE_DIR}/tools/make/download_dependencies.sh
else
    echo "Dependencies already downloaded"
fi

pushd ${TF_LITE_DIR}

if [[ "$BUILD_TYPE" == "native" ]]; then
  make -j 4 \
      VERBOSE=1 \
      MINIMAL_SRCS= \
      BUILD_WITH_NNAPI=OFF \
      -C "${TF_DIR}" \
      -f tensorflow/lite/tools/make/Makefile
else
  source ${FAASM_TOOLCHAIN}/env.sh

  LIBS="-lstdc++ -ldlmalloc"

  make -j 4 \
      VERBOSE=1 \
      CC=${WASM_CC} \
      CXX=${WASM_CXX} \
      CPP=${WASM_CPP} \
      AR=${WASM_AR} \
      RANLIB=${WASM_RANLIB} \
      CFLAGS="${WASM_CFLAGS} -ftls-model=local-exec -ldlmalloc" \
      CXXFLAGS="${WASM_CXXFLAGS} -ldlmalloc" \
      CPPFLAGS="${WASM_CPPFLAGS}" \
      LDFLAGS="${WASM_LDFLAGS} -Xlinker --no-check-features -Xlinker --max-memory=1073741824" \
      MINIMAL_SRCS= \
      TARGET=${WASM_HOST} \
      TARGET_ARCH=${WASM_TARGET} \
      LIBS=${LIBS} \
      LDOPTS=${LDOPTS} \
      BUILD_WITH_NNAPI=false \
      BUILD_WITH_MMAP=false \
      -C "${TF_DIR}" \
      -f tensorflow/lite/tools/make/Makefile
fi

popd
