#!/bin/bash
set -e

THIS_DIR=$(dirname $(readlink -f $0))
PROJ_ROOT=${THIS_DIR}/../..
pushd ${PROJ_ROOT} > /dev/null

export FAASM_BUILD_MOUNT=/build/faasm
export FAASM_LOCAL_MOUNT=/usr/local/faasm

if [[ -z "$1" ]]; then
    docker-compose up -d dist-test-server
elif [[ "$1" == "restart" ]]; then
    docker-compose restart dist-test-server
elif [[ "$1" == "stop" ]]; then
    docker-compose stop dist-test-server
else
    echo "Unrecognised argument: $1"
    echo ""
    echo "Usage:"
    echo ""
    echo "./deploy/dist-test/dev_server.sh [restart|stop]"
    exit 1
fi

popd > /dev/null
