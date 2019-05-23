#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))

BUILD_DIR=${THIS_DIR}/../func/build_native
mkdir -p ${BUILD_DIR}

pushd ${BUILD_DIR} >> /dev/null

cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --target polybench_all_funcs

popd