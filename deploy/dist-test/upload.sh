#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
export PROJ_ROOT=${THIS_DIR}/../..
pushd ${PROJ_ROOT} > /dev/null

export FAASM_BUILD_MOUNT=/build/faasm
export FAASM_CODE_MOUNT=/usr/local/code/faasm
export FAASM_CONAN_MOUNT=/root/.conan

# Make sure upload server is running
docker compose \
    up \
    -d \
    upload

# Give the upload server time to start
sleep 5

# Run the function build and upload
docker compose \
    run \
    --rm \
    cpp \
    ./bin/inv_wrapper.sh func.user demo \
    func.upload-user demo \
    func.user mpi \
    func.upload-user mpi \
    func.user omp \
    func.upload-user omp

popd >> /dev/null
