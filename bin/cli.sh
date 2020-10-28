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
echo "Running Faasm CLI (${CLI_IMAGE})"

# Run shell in CLI container
docker-compose \
    -f docker/docker-compose-cli.yml \
    run \
    cli \
    ${INNER_SHELL}

popd > /dev/null
