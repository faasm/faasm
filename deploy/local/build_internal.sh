#!/bin/bash

set -e

export PROJ_ROOT=$(dirname $(dirname $(readlink -f $0)))
pushd ${PROJ_ROOT} >> /dev/null

# Run the build
inv -r faasmcli/faasmcli dev.cmake --build=Debug
inv -r faasmcli/faasmcli dev.tools

popd >> /dev/null
