#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
export PROJ_ROOT=${THIS_DIR}/../..
pushd ${PROJ_ROOT} > /dev/null

# Run the build
inv -r faasmcli/faasmcli dev.cmake --build=Debug
inv -r faasmcli/faasmcli dev.cc dist_tests
inv -r faasmcli/faasmcli dev.cc dist_test_server

popd >> /dev/null
