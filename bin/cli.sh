#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
PROJ_ROOT=${THIS_DIR}/..

pushd ${PROJ_ROOT} > /dev/null

# Default CLI image
VERSION=$(cat VERSION)
DEFAULT_IMAGE=faasm/cli:${VERSION}
export CLI_IMAGE=${1:-${DEFAULT_IMAGE}}
echo "Running Faasm CLI (${CLI_IMAGE})"

# Start docker-compose in the background
docker-compose \
    -f docker/docker-compose-cli.yml \
    up \
    -d

# Run prompt in CLI container
docker-compose \
    -f docker/docker-compose-cli.yml \
    run \
    cli \
    /bin/bash

popd > /dev/null
