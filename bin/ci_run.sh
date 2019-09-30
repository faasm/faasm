#!/bin/bash

set -e

pushd /usr/local/code/faasm

source workon.sh

# Set up libfake for tests
inv compile-libfake --clean

# Run codegen on all files used locally
inv run-local-codegen

popd

echo "Execute tests"
/faasm/build/bin/tests
