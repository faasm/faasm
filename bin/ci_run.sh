#!/bin/bash

set -e

pushd /usr/local/code/faasm

# Set up cgroup and namespaces
./bin/cgroup.sh
./bin/netns.sh 5

# Download the toolchain
inv download-toolchain

# Set up libfake for tests
inv compile-libfake --clean

# Run codegen on all functions held locally
inv run-local-codegen

# Set Python and the runtime root
inv set-up-python-runtime
inv upload-all --py --host=upload

# Set up TF data
inv set-up-tensorflow-data --host=upload

popd

echo "Execute tests"
/faasm/build/bin/tests
