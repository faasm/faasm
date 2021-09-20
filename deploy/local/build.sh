#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
export PROJ_ROOT=${THIS_DIR}/../..

pushd ${PROJ_ROOT} >> /dev/null

# Run the build
docker-compose \
    run \
    faasm-cli \
    /usr/local/code/faasm/deploy/local/build_internal.sh

popd >> /dev/null
