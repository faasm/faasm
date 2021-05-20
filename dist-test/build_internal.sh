#!/bin/bash

set -e

export PROJ_ROOT=$(dirname $(dirname $(readlink -f $0)))
pushd ${PROJ_ROOT} >> /dev/null

# Run the build
inv -r faasmcli/faasmcli dev.cmake --build=Debug
inv -r faasmcli/faasmcli dev.cc dist_tests
inv -r faasmcli/faasmcli dev.cc dist_test_server

# Copy the results
cp -r /build/faasm/* /build/dist-test/

popd >> /dev/null
