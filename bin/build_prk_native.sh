#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
PROJ_ROOT=${THIS_DIR}/..

FAASM_HOME=${HOME}/faasm
WORK_DIR=${FAASM_HOME}/ParResKernels

REPO=https://github.com/Shillaker/Kernels

if [[ ! -d "$WORK_DIR" ]]; then
  # Check out the PRK code
  mkdir -p ${FAASM_HOME}
  pushd ${FAASM_HOME} > /dev/null

  git clone ${REPO} ParResKernels

  popd > /dev/null
fi

pushd ${WORK_DIR}

cp common/make.defs.faasm_native common/make.defs
make allmpi -j

popd > /dev/null
