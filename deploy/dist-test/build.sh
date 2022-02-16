#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
export PROJ_ROOT=${THIS_DIR}/../..
pushd ${PROJ_ROOT} >> /dev/null

export FAASM_BUILD_MOUNT=/build/faasm
export CONAN_CACHE_MOUNT_SOURCE=$HOME/.conan/
# Ensure the cache directory exists before starting the containers
mkdir -p ${CONAN_CACHE_MOUNT_SOURCE}

# Run the build
docker-compose \
    run \
    --rm \
    faasm-cli \
    /usr/local/code/faasm/deploy/dist-test/build_internal.sh

popd >> /dev/null
