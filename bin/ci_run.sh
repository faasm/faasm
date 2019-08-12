#!/bin/bash

set -e

pushd /usr/local/code/faasm
echo "Set up local tooling"
source workon.sh
./bin/set_up_libfake.sh
popd

echo "Run codegen on wasm files"
/faasm/build/bin/codegen /usr/local/code/faasm/wasm/demo
/faasm/build/bin/codegen /usr/local/code/faasm/wasm/errors
/faasm/build/bin/codegen /usr/local/code/faasm/wasm/python
/faasm/build/bin/codegen /usr/local/code/faasm/wasm/sgd

echo "Run codegen on python files"
/faasm/build/bin/codegen /usr/local/faasm/runtime_root/lib/python3.7

echo "Execute tests"
/faasm/build/bin/tests

popd
