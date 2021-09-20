#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
export PROJ_ROOT=${THIS_DIR}/../..

pushd ${PROJ_ROOT} >> /dev/null

# Set up image name
export FAASM_VERSION=$(cat VERSION)
if [[ -z "${FAASM_CLI_IMAGE}" ]]; then
    export FAASM_CLI_IMAGE=faasm/cli:${FAASM_VERSION}
fi

# Run the build
docker-compose \
    run \
    faasm-cli \
    /usr/local/code/faasm/dist-test/build_internal.sh

popd >> /dev/null
