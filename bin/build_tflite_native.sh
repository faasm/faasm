#!/bin/bash

set -e

# Runs native build of TF Lite

if [[ "${FAASM_USER}" != "tf" ]]; then
  echo "FAASM_USER not set to tf, so skipping (current = ${FAASM_USER})"
  exit 0
fi

THIS_DIR=$(dirname $(readlink -f $0))
TF_DIR=${THIS_DIR}/../third-party/tensorflow
TF_LITE_DIR=${TF_DIR}/tensorflow/lite

pushd ${TF_LITE_DIR}

make -j \
  BUILD_WITH_NNAPI=false \
  -C ${TF_DIR} \
  -f tensorflow/lite/tools/make/Makefile \
  lib

popd
