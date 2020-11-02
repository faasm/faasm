#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
PROJ_ROOT=${THIS_DIR}/..

pushd ${PROJ_ROOT} > /dev/null

# Default CLI image
VERSION=$(cat VERSION)
DEFAULT_IMAGE=faasm/cli:${VERSION}
INNER_SHELL=${SHELL:-"/bin/bash"}
export CLI_IMAGE=${1:-${DEFAULT_IMAGE}}
export COMPOSE_PROJECT_NAME="faasm-dev"
echo "Running Faasm CLI (${CLI_IMAGE})"

## Run shell in CLI container
# If service not running, bring it up first.
# If the services are already running, images won't be recreated as we are
# using the --no-recreate flag.
docker-compose \
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
