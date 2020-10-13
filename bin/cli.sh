#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))

pushd ${THIS_DIR}/.. > /dev/null

VERSION=$(cat VERSION)

docker run --entrypoint="/bin/bash" \
    --network="host" \
    -e "TERM=xterm-256color" \
    -v $(pwd):/usr/local/code/faasm \
    -w /usr/local/code/faasm \
    -it \
    faasm/cli:${VERSION}

popd > /dev/null
