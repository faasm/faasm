#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
PROJ_ROOT=${THIS_DIR}/..

pushd ${PROJ_ROOT} > /dev/null

# Stop all faasm-related containers
echo "Stopping Faasm services"
docker-compose -p faasm-dev -f docker/docker-compose-cli.yml stop

popd > /dev/null

