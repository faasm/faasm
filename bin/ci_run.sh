#!/bin/bash

set -e

pushd /usr/local/code/faasm
echo "Set up libfake"
source workon.sh

inv compile-libfake --clean

inv run-local-codegen

popd

echo "Execute tests"
/faasm/build/bin/tests
