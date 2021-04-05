#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
PROJ_ROOT=${THIS_DIR}/..

pushd ${PROJ_ROOT} > /dev/null

export FAASM_VERSION=$(cat VERSION)
export CPP_VERSION=$(cat clients/cpp/VERSION)

# Mount our local build into the local cluster
export FAASM_BUILD_MOUNT=/build/faasm

if [[ -z "$FAASM_CLI_IMAGE" ]]; then
    export FAASM_CLI_IMAGE=faasm/cli:${FAASM_VERSION}
fi

if [[ -z "$CPP_CLI_IMAGE" ]]; then
    export CPP_CLI_IMAGE=faasm/cpp-sysroot:${CPP_VERSION}
fi

INNER_SHELL=${SHELL:-"/bin/bash"}

docker-compose -f docker-compose.yml \
    up \
    --no-recreate \
    -d \
    faasm-cli

docker-compose -f docker-compose.yml \
    exec \
    faasm-cli \
    ${INNER_SHELL}

popd > /dev/null
