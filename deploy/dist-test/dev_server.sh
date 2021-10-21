#!/bin/bash
set -e

THIS_DIR=$(dirname $(readlink -f $0))
PROJ_ROOT=${THIS_DIR}/../..
pushd ${PROJ_ROOT} > /dev/null

export FAASM_BUILD_MOUNT=/build/faasm
export FAASM_LOCAL_MOUNT=/usr/local/faasm

if [[ -z "$1" ]]; then
    docker-compose \
        up \
        -d \
        dist-test-server
else
    docker-compose \
        restart \
        dist-test-server
fi

popd > /dev/null
