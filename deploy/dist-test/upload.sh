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

# Give the upload server time to start
sleep 5

# Run the function build and upload
docker-compose \
    run \
    --rm \
    cpp \
    inv func.user demo \
    func.upload-user demo \
    func mpi mpi_bcast \
    func.upload mpi mpi_bcast \
    func.user omp \
    func.upload-user omp

popd >> /dev/null
