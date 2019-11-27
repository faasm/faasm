#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))

pushd ${THIS_DIR}/.. > /dev/null

VERSION=$(cat VERSION)

docker run --entrypoint="/bin/bash" --network="host" -v $(pwd):/usr/local/code/faasm -w /usr/local/code/faasm -it faasm/toolchain:${VERSION}

popd > /dev/null
