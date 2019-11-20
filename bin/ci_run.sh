#!/bin/bash

set -e

pushd /usr/local/code/faasm

# Set up cgroup and namespaces
./bin/cgroup.sh
./bin/netns.sh 5

# Download the toolchain
inv download-toolchain
inv download-sysroot
inv download-runtime-root

# Set up libfake for tests
inv compile-libfake --clean

# Run codegen on local stuff
inv run-local-codegen

# Set Python and the runtime root
inv upload-all --py --local-copy

# Set up TF data
inv set-up-tensorflow-data --local-copy

popd

echo "Execute tests"
/faasm/build/bin/tests
