#!/bin/bash

set -e

if [[ -z "$1" ]]; then
    RELEASE_TYPE=Release
else
    RELEASE_TYPE=$1
fi

echo "Running release type ${RELEASE_TYPE}"

THIS_DIR=$(dirname $(readlink -f $0))
PROJ_ROOT=${THIS_DIR}/..
TOOLCHAIN=${PROJ_ROOT}/toolchain/ClangNativeToolchain.cmake

BUILD_DIR=${PROJ_ROOT}/build/polybench_native
rm -rf ${BUILD_DIR}
mkdir -p ${BUILD_DIR}

pushd ${BUILD_DIR} >> /dev/null

cmake -DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN} ${PROJ_ROOT}
cmake --build . --target polybench_all_funcs  -- -j

popd
