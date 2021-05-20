#!/bin/bash

set -e

export PROJ_ROOT=$(dirname $(dirname $(readlink -f $0)))
DIST_TEST_DIR=${PROJ_ROOT}/dist-test
pushd ${DIST_TEST_DIR} >> /dev/null

# Set up image name
if [[ -z "${FAASM_CLI_IMAGE}" ]]; then
    VERSION=$(cat ../VERSION)
    export FAASM_CLI_IMAGE=faasm/cli:${VERSION}
fi

if [ "$1" == "local" ]; then
    INNER_SHELL=${SHELL:-"/bin/bash"}

    # Assume default dev set-up
    export FAASM_LOCAL_DIR=${PROJ_ROOT}/dev/faasm-local

    # Start everything in the background
    docker-compose \
        up \
        --no-recreate \
        -d \
        master

    # Run the CLI
    docker-compose \
        exec \
        master \
        ${INNER_SHELL}
else
    # Use the faasm-local dir set up by the build
    export FAASM_LOCAL_DIR=${DIST_TEST_DIR}/build/faasm-local

    # Run the tests directly
    docker-compose \
        run \
        master \
        /build/faasm/bin/dist_tests

    docker-compose \
        stop
fi

popd >> /dev/null
