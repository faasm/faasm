#!/bin/bash

set -e

export PROJ_ROOT=$(dirname $(dirname $(readlink -f $0)))
pushd ${PROJ_ROOT} >> /dev/null

# Copy the contents of the container's local directory into the shared mount
echo "Copying local dir into place"
cp -r /usr/local/faasm/* /build/faasm-local/

# Run the build
inv -r faasmcli/faasmcli dev.cmake --build=Debug
inv -r faasmcli/faasmcli dev.cc upload
inv -r faasmcli/faasmcli dev.cc dist_tests
inv -r faasmcli/faasmcli dev.cc dist_test_server

# Copy the results
echo "Copying build outputs into place"
cp -r /build/faasm/* /build/dist-test/

popd >> /dev/null
