#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
export PROJ_ROOT=${THIS_DIR}/../..

pushd ${PROJ_ROOT} > /dev/null

export FAASM_VERSION=$(cat VERSION)

# Set up image name
export CPP_VERSION=$(cat clients/cpp/VERSION)
if [[ -z "$CPP_CLI_IMAGE" ]]; then
    export CPP_CLI_IMAGE=faasm/cpp-sysroot:${CPP_VERSION}
fi

# Run the function build and upload
docker-compose \
    run \
    cpp \
    inv func.user demo func.user omp func.upload-user demo func.upload-user omp

popd >> /dev/null
