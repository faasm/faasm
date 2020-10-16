#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
PROJ_ROOT=${THIS_DIR}/..

pushd ${PROJ_ROOT} > /dev/null

VERSION=$(cat VERSION)

docker run \
    --entrypoint="/bin/bash" \
    --network="host" \
    -v $(pwd):/usr/local/code/faasm \
    -w /usr/local/code/faasm \
    -it \
    faasm/cli:${VERSION}

popd > /dev/null
