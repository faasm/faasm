#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
PROJ_ROOT=${THIS_DIR}/..

pushd ${PROJ_ROOT} > /dev/null

# See if someone has specified a CLI image through an environment variable
if [[ -z "${CLI_IMAGE}" ]]; then
    # Prepare the default version
    VERSION=$(cat VERSION)
    DEFAULT_IMAGE=faasm/cli:${VERSION}

    # Take the script argument if provided, else use the default
    export CLI_IMAGE=${1:-${DEFAULT_IMAGE}}
fi

echo "Running Faasm CLI (${CLI_IMAGE})"

INNER_SHELL=${SHELL:-"/bin/bash"}

export COMPOSE_PROJECT_NAME="faasm-dev"

## Run shell in CLI container
# If service not running, bring it up first.
# If the services are already running, images won't be recreated as we are
# using the --no-recreate flag.
docker-compose \
    -p ${COMPOSE_PROJECT_NAME} \
    -f docker/docker-compose-cli.yml \
    up \
    --no-recreate \
    -d

# Then attach to it (note we `up` containers in dettached mode).
docker-compose \
    -p ${COMPOSE_PROJECT_NAME} \
    -f docker/docker-compose-cli.yml \
    exec \
    cli \
    ${INNER_SHELL}

popd > /dev/null
