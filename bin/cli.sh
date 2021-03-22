#!/bin/bash
set -e

THIS_DIR=$(dirname $(readlink -f $0))
PROJ_ROOT=${THIS_DIR}/..

pushd ${PROJ_ROOT} > /dev/null

export FAABRIC_VERSION=$(cat faabric/VERSION)
export FAASM_VERSION=$(cat VERSION)
export CPP_VERSION=$(cat clients/cpp/VERSION)
export PYTHON_VERSION=$(cat clients/python/VERSION)

if [[ -z "$FAABRIC_CLI_IMAGE" ]]; then
    export FAABRIC_CLI_IMAGE=faasm/faabric:${FAABRIC_VERSION}
fi

if [[ -z "$FAASM_CLI_IMAGE" ]]; then
    export FAASM_CLI_IMAGE=faasm/cli:${FAASM_VERSION}
fi

if [[ -z "$CPP_CLI_IMAGE" ]]; then
    export CPP_CLI_IMAGE=faasm/cpp-sysroot:${CPP_VERSION}
fi

if [[ -z "$PYTHON_CLI_IMAGE" ]]; then
    export PYTHON_CLI_IMAGE=faasm/cpython:${PYTHON_VERSION}
fi

if [[ -z "$1" ]]; then
    echo "Must specify which CLI"
    exit 1

elif [[ "$1" == "faabric" ]]; then
    MODE="faabric"
    CLI_CONTAINER="faabric-cli"
    echo "Faabric CLI (${FAABRIC_CLI_IMAGE})"

elif [[ "$1" == "faasm" ]]; then
    MODE="faasm"
    CLI_CONTAINER="faasm-cli"
    echo "Faasm CLI (${FAASM_CLI_IMAGE})"

elif [[ "$1" == "cpp" ]]; then
    MODE="cpp"
    CLI_CONTAINER="cpp-cli"
    echo "CPP CLI (${CPP_CLI_IMAGE})"

elif [[ "$1" == "python" ]]; then
    MODE="python"
    CLI_CONTAINER="python-cli"
    echo "Python CLI (${PYTHON_CLI_IMAGE})"

else
    echo "Unrecognised CLI. Must be one of faabric, faasm, cpp or python"
    exit 1
fi

INNER_SHELL=${SHELL:-"/bin/bash"}

# Make sure the CLI is running already in the background (avoids creating a new
# container every time)
docker-compose -f docker-compose-dev.yml \
    up \
    --no-recreate \
    -d \
    ${CLI_CONTAINER}    

# Attach to the CLI container
docker-compose -f docker-compose-dev.yml \
    exec \
    ${CLI_CONTAINER} \
    ${INNER_SHELL}

popd > /dev/null
