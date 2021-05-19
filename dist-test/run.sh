#!/bin/bash

set -e

export PROJ_ROOT=$(dirname $(dirname $(readlink -f $0)))
pushd ${PROJ_ROOT}/dist-test >> /dev/null

# Set up image name
if [[ -z "${FAASM_CLI_IMAGE}" ]]; then
    VERSION=$(cat ../VERSION)
    export FAASM_CLI_IMAGE=faasm/cli:${VERSION}
fi

if [ "$1" == "local" ]; then
    INNER_SHELL=${SHELL:-"/bin/bash"}

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
    # Run the tests directly
    docker-compose \
        run \
        master \
        /build/faasm/bin/dist_tests

    docker-compose \
        stop
fi

popd >> /dev/null
