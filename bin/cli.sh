#!/bin/bash
set -e

THIS_DIR=$(dirname $(readlink -f $0))
PROJ_ROOT=${THIS_DIR}/..

pushd ${PROJ_ROOT} > /dev/null

function usage() {
    echo "Usage: "
    echo "./bin/cli.sh <container>"
    echo ""
    echo "container being one of: "
    echo "- cpp             C/C++ functions"
    echo "- faasm           Managing Faasm cluster"
    echo "- faasm-sgx-sim   Manage SGX-enabled Faasm cluster (simulation mode)"
    echo "- faasm-sgx       Manage SGX-enabled Faasm cluster"
    echo "- python          Python functions"
}

# The AESMD socket is necessary when running SGX in hardware mode for remote
# attestation. In a K8s deployment it is provided by the K8s runtime.
function start_sgx_aesmd_socket() {
    # First, double check the SGX devices exist in the host
    if [ ! -d /dev/sgx ] ; then
        echo "SGX hardware mode set, but SGX devices not found under /dev/sgx"
        exit 1
    fi
    export SGX_DEVICE_MOUNT_DIR=/dev/sgx
    # Create a named volume to communicate between containers using sockets
    export AESMD_SOCKET_EXTERNAL_VOLUME=true
    docker volume create \
        --driver local \
        --opt type=tmpfs \
        --opt device=tmpfs \
        --opt o=rw \
        aesmd-socket
    docker compose \
        up \
        --no-recreate \
        -d \
        aesmd
}

if [[ -z "$1" ]]; then
    usage
    exit 1
elif [[ "$1" == "faasm" ]]; then
    CLI_CONTAINER="faasm-cli"
elif [[ "$1" == "faasm-sgx-sim" ]]; then
    CLI_CONTAINER="faasm-cli"
    export FAASM_CLI_IMAGE=faasm/cli-sgx-sim:$(cat ${PROJ_ROOT}/VERSION)
    export FAASM_WORKER_IMAGE=faasm/worker-sgx-sim:$(cat ${PROJ_ROOT}/VERSION)
    export WASM_VM=sgx
elif [[ "$1" == "faasm-sgx" ]]; then
    CLI_CONTAINER="faasm-cli"
    export FAASM_CLI_IMAGE=faasm/cli-sgx:$(cat ${PROJ_ROOT}/VERSION)
    export FAASM_WORKER_IMAGE=faasm/worker-sgx:$(cat ${PROJ_ROOT}/VERSION)
    export WASM_VM=sgx
    start_sgx_aesmd_socket
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
export FAASM_BUILD_MOUNT=/build/faasm
export FAASM_LOCAL_MOUNT=/usr/local/faasm

# Make sure the CLI is running already in the background (avoids creating a new
# container every time)
docker compose \
    up \
    --no-recreate \
    -d \
    ${CLI_CONTAINER}

# Attach to the CLI container
docker compose \
    exec \
    ${CLI_CONTAINER} \
    ${INNER_SHELL}

popd > /dev/null
