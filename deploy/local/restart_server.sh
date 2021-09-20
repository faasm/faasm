#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
export PROJ_ROOT=${THIS_DIR}/../..

pushd ${PROJ_ROOT} >> /dev/null

# Restart the worker
docker-compose \
    restart \
    dist-test-server

popd >> /dev/null
