#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
PROJ_ROOT=${THIS_DIR}/..

pushd ${PROJ_ROOT} > /dev/null

# See if someone has specified a CLI image through an environment variable
if [[ -z "${FAASM_CLI_IMAGE}" ]]; then
    # Prepare the default version
    FAASM_VERSION=$(cat VERSION)
    DEFAULT_IMAGE=faasm/cli:${FAASM_VERSION}

    # Take the script argument if provided, else use the default
    export FAASM_CLI_IMAGE=${1:-${DEFAULT_IMAGE}}
fi

echo "Running Faasm CLI (${FAASM_CLI_IMAGE})"

INNER_SHELL=${SHELL:-"/bin/bash"}

# Make sure the CLI is running already in the background (avoids creating a new
# container every time)
docker-compose \
    up \
    --no-recreate \
    -d \
    cli 

# Attach to the CLI container
docker-compose \
    exec \
    cli \
    ${INNER_SHELL}

popd > /dev/null
