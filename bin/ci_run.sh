#!/bin/bash

set -e

pushd /usr/local/code/faasm
echo "Set up libfake"
source workon.sh
inv compile-libfake --clean
popd

echo "Run codegen on wasm files"
/faasm/build/bin/codegen_func demo
/faasm/build/bin/codegen_func errors

# Run these in parallel
/faasm/build/bin/codegen_func python &
/faasm/build/bin/codegen_func sgd &
/faasm/build/bin/codegen_func tf &

wait

echo "Run codegen on python files"
/faasm/build/bin/codegen_shared_obj /usr/local/faasm/runtime_root/lib/python3.7

echo "Execute tests"
/faasm/build/bin/tests
