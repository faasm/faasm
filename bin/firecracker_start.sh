#!/bin/bash

set -e

FIRECRACKER_DIR=${HOME}/faasm/firecracker
API_SOCKET=/tmp/firecracker.socket

# Need to make sure
rm -f ${API_SOCKET}

pushd ${FIRECRACKER_DIR} >> /dev/null

# Start firecracker
./firecracker --api-sock ${API_SOCKET}

popd >> /dev/null