#!/bin/bash

set -e

export PROJ_ROOT=$(dirname $(dirname $(readlink -f $0)))
pushd ${PROJ_ROOT}/dist-test >> /dev/null

# Set up image name
if [[ -z "${FAASM_CLI_IMAGE}" ]]; then
    VERSION=$(cat ../VERSION)
    export FAASM_CLI_IMAGE=faasm/cli:${VERSION}
fi

RETURN_VAL=0

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
    export FAASM_LOCAL_DIR=${PROJ_ROOT}/dist-test/build/faasm-local

    # Run the tests directly
    docker-compose \
        run \
        --rm \
        master \
        /build/faasm/bin/dist_tests
    RETURN_VAL=$?

    echo "-------------------------------------------"
    echo "                WORKER LOGS                "
    echo "-------------------------------------------"
    docker-compose logs worker

    # Stop everything
    docker-compose stop
fi

popd >> /dev/null

exit $RETURN_VAL
