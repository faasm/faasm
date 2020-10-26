#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
PROJ_ROOT=${THIS_DIR}/..

pushd ${PROJ_ROOT} > /dev/null

# Default CLI image
VERSION=$(cat VERSION)
DEFAULT_IMAGE=faasm/cli:${VERSION}
INNER_SHELL=${SHELL:-"/bin/bash"}
CLI_NAME="docker_faasm_cli"
export CLI_IMAGE=${1:-${DEFAULT_IMAGE}}
echo "Running Faasm CLI (${CLI_IMAGE})"

## Run shell in CLI container
# If already running, attach a new shell to it
# otherwise, spin a new container
if [[ $(docker ps -q --filter name=${CLI_NAME}) ]]; then
    docker exec -it \
        ${CLI_NAME} \
        ${INNER_SHELL}
else
    docker-compose \
        -f docker/docker-compose-cli.yml \
        run \
        --name ${CLI_NAME} \
        cli \
        ${INNER_SHELL}
fi

popd > /dev/null
