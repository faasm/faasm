#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
export PROJ_ROOT=${THIS_DIR}/../..
pushd ${PROJ_ROOT} > /dev/null

export FAASM_BUILD_MOUNT=/build/faasm

# Make sure upload server is running
docker-compose \
    up \
    -d \
    upload

# Wait for it to be up
./deploy/local/wait_for_upload.sh localhost 8002

# Run the function build and upload
docker-compose \
    run \
    --rm \
    cpp \
    inv func.user demo \
    func.upload-user demo \
    func.user omp \
    func.upload-user omp

popd >> /dev/null
