#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
export PROJ_ROOT=${THIS_DIR}/../..

pushd ${PROJ_ROOT} > /dev/null

# Run the function build and upload
docker-compose \
    run \
    --rm \
    cpp \
    inv func.user demo func.user omp func.upload-user demo func.upload-user omp

popd >> /dev/null
