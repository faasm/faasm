#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
export PROJ_ROOT=${THIS_DIR}/../..

pushd ${PROJ_ROOT}/dist-test >> /dev/null

export FAASM_VERSION=$(cat VERSION)

# Restart the worker
docker-compose \
    restart \
    dist-test-server

popd >> /dev/null
