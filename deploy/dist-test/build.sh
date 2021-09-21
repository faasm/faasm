#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
export PROJ_ROOT=${THIS_DIR}/../..
pushd ${PROJ_ROOT} >> /dev/null

# Note that we need to share the build directory in the distributed tests
export FAASM_BUILD_MOUNT=/build/faasm

# Run the build
docker-compose \
    run \
    --rm \
    faasm-cli \
    /usr/local/code/faasm/deploy/dist-test/build_internal.sh

popd >> /dev/null
