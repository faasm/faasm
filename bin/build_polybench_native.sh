#!/bin/bash

set -e

if [[ -z "$1" ]]; then
    RELEASE_TYPE=Release
else
    RELEASE_TYPE=$1
fi

echo "Running release type ${RELEASE_TYPE}"

THIS_DIR=$(dirname $(readlink -f $0))

BUILD_DIR=${THIS_DIR}/../build/polybench_native
rm -rf ${BUILD_DIR}
mkdir -p ${BUILD_DIR}

pushd ${BUILD_DIR} >> /dev/null

cmake -DCMAKE_BUILD_TYPE=${RELEASE_TYPE} ..
cmake --build . --target polybench_all_funcs  -- -j

popd