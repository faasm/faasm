#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
export PROJ_ROOT=${THIS_DIR}/../..

pushd ${PROJ_ROOT} > /dev/null

# Mount our local build into the local cluster
export FAASM_BUILD_MOUNT=/build/faasm

INNER_SHELL=${SHELL:-"/bin/bash"}

docker-compose -f docker-compose.yml \
    up \
    --no-recreate \
    -d \
    --scale worker=2

if [[ "$1" == "faasm" ]]; then
    CLI_CONTAINER="faasm-cli"
elif [[ "$1" == "cpp" ]]; then
    CLI_CONTAINER="cpp"
elif [[ "$1" == "python" ]]; then
    CLI_CONTAINER="python"
else
    echo "Not starting a CLI container"
    exit 0
fi

echo "Starting shell in ${CLI_CONTAINER}"

docker-compose -f docker-compose.yml \
    exec \
    ${CLI_CONTAINER} \
    ${INNER_SHELL}

popd > /dev/null
