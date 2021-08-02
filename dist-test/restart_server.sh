#!/bin/bash

set -e

export FAASM_VERSION=$(cat ../VERSION)
export PROJ_ROOT=$(dirname $(dirname $(readlink -f $0)))
pushd ${PROJ_ROOT}/dist-test >> /dev/null

# Restart the worker
docker-compose \
    restart \
    worker

popd >> /dev/null
