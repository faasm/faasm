#!/bin/bash

set -e

export PROJ_ROOT=$(dirname $(dirname $(readlink -f $0)))
pushd ${PROJ_ROOT}/dist-test >> /dev/null

# Run the build
inv dev.cmake
inv dev.cc dist_tests
inv dev.cc dist_test_server

# Copy the results
cp -r /build/faasm/* /build/dist-test/

popd >> /dev/null
