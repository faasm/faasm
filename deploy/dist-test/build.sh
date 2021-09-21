#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
export PROJ_ROOT=${THIS_DIR}/../..

export FAASM_BUILD_MOUNT=/build/faasm

pushd ${PROJ_ROOT} >> /dev/null

# Run the build
docker-compose \
    run \
    --rm \
    faasm-cli \
    /usr/local/code/faasm/deploy/dist-test/build_internal.sh

popd >> /dev/null
