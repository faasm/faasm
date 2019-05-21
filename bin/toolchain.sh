#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))

pushd ${THIS_DIR}/.. > /dev/null

docker run -v $(pwd):/usr/local/code/faasm -w /usr/local/code/faasm -it faasm/toolchain

popd > /dev/null