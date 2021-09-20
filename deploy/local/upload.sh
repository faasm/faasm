#!/bin/bash

set -e

export PROJ_ROOT=$(dirname $(dirname $(readlink -f $0)))
pushd ${PROJ_ROOT}/dist-test >> /dev/null

export FAASM_VERSION=$(cat ../VERSION)

# Set up image name
export CPP_VERSION=$(cat ../clients/cpp/VERSION)
if [[ -z "$CPP_CLI_IMAGE" ]]; then
    export CPP_CLI_IMAGE=faasm/cpp-sysroot:${CPP_VERSION}
fi

# Use the faasm-local dir set up by the build
export FAASM_LOCAL_DIR=${PROJ_ROOT}/dist-test/build/faasm-local

# Run the function build and upload
docker-compose \
    run \
    cpp \
    inv func.user demo func.user omp func.upload-user demo func.upload-user omp

popd >> /dev/null
