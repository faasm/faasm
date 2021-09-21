#!/bin/bash
set -e

THIS_DIR=$(dirname $(readlink -f $0))
PROJ_ROOT=${THIS_DIR}/../..
pushd ${PROJ_ROOT} > /dev/null

export FAASM_BUILD_MOUNT=/faasm/build
export FAASM_LOCAL_MOUNT=/usr/local/faasm

docker-compose \
    up \
    -d \
    --scale worker=2 \
    nginx

popd > /dev/null
