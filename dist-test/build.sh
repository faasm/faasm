#!/bin/bash

set -e

export PROJ_ROOT=$(dirname $(dirname $(readlink -f $0)))
pushd ${PROJ_ROOT}/dist-test >> /dev/null

export FAASM_VERSION=$(cat ../VERSION)

# Set up image name
if [[ -z "${FAASM_CLI_IMAGE}" ]]; then
    export FAASM_CLI_IMAGE=faasm/cli:${FAASM_VERSION}
fi

# Run the build
docker-compose \
    run \
    --rm \
    builder \
    /usr/local/code/faasm/dist-test/build_internal.sh

popd >> /dev/null
