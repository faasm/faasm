#!/bin/bash

set -e

export PROJ_ROOT=$(dirname $(dirname $(readlink -f $0)))
pushd ${PROJ_ROOT}/dist-test >> /dev/null

export FAASM_VERSION=$(cat ../VERSION)
export CPP_VERSION=$(cat ../clients/cpp/VERSION)

# Set up image names
if [[ -z "${FAASM_CLI_IMAGE}" ]]; then
    export FAASM_CLI_IMAGE=faasm/cli:${FAASM_VERSION}
fi

if [[ -z "$CPP_CLI_IMAGE" ]]; then
    export CPP_CLI_IMAGE=faasm/cpp-sysroot:${CPP_VERSION}
fi

# Run the build
docker-compose \
    run \
    builder \
    /usr/local/code/faasm/dist-test/build_internal.sh

# Run the function build and upload
docker-compose \
    run \
    cpp \
    inv func.user demo func.user omp func.upload-user demo func.upload-user omp

popd >> /dev/null
