#!/bin/bash
set -e

THIS_DIR=$(dirname $(readlink -f $0))
PROJ_ROOT=${THIS_DIR}/..

pushd ${PROJ_ROOT} > /dev/null

export FAABRIC_VERSION=$(cat faabric/VERSION)
export FAASM_VERSION=$(cat VERSION)
export CPP_VERSION=$(cat clients/cpp/VERSION)
export PYTHON_VERSION=$(cat clients/python/VERSION)

function usage() {
    echo "Usage: "
    echo "./bin/cli.sh <container>"
    echo ""
    echo "container being one of: "
    echo "- cpp         C/C++ functions"
    echo "- faabric     Faabric development"
    echo "- faasm       Managing Faasm cluster"
    echo "- python      Python functions"
}

if [[ -z "$1" ]]; then
    usage
    exit 1
elif [[ "$1" == "faabric" ]]; then
    CLI_CONTAINER="faabric"
elif [[ "$1" == "faasm" ]]; then
    CLI_CONTAINER="faasm-cli"
elif [[ "$1" == "cpp" ]]; then
    CLI_CONTAINER="cpp"
elif [[ "$1" == "python" ]]; then
    CLI_CONTAINER="python"
else
    usage
    exit 1
fi

INNER_SHELL=${SHELL:-"/bin/bash"}

# This is how we ensure the development mode is on, mounting our local
# directories into the containers to override what's already there
export FAASM_BUILD_MOUNT=/faasm/build
export FAASM_LOCAL_MOUNT=/usr/local/faasm

# Make sure the CLI is running already in the background (avoids creating a new
# container every time)
docker-compose -f docker-compose.yml \
    up \
    --no-recreate \
    -d \
    ${CLI_CONTAINER}

# Attach to the CLI container
docker-compose -f docker-compose.yml \
    exec \
    ${CLI_CONTAINER} \
    ${INNER_SHELL}

popd > /dev/null
