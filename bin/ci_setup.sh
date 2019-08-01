#!/bin/bash

set -e

mkdir -p /faasm/ci-build
pushd /faasm/ci-build

echo "Configure out of source build"
cmake -DCMAKE_CXX_COMPILER=/usr/bin/clang++ -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_BUILD_TYPE=Debug /usr/local/code/faasm

echo "Build codegen"
cmake --build . --target codegen

echo "Run codegen on wasm files"
./bin/codegen /usr/local/code/faasm/wasm/demo
./bin/codegen /usr/local/code/faasm/wasm/errors
./bin/codegen /usr/local/code/faasm/wasm/python
./bin/codegen /usr/local/code/faasm/wasm/sgd

echo "Build tests"
cmake --build . --target tests

popd
