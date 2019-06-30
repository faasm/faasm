#!/bin/bash

set -e

pushd /faasm/build

echo "Configure out of source build"
cmake -DCMAKE_CXX_COMPILER=/usr/bin/clang++ -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_BUILD_TYPE=Debug /usr/local/code/faasm

echo "Build codegen"
cmake --build . --target codegen

echo "Run codegen on wasm files"
./bin/codegen /usr/local/code/faasm/wasm

echo "Build tests"
cmake --build . --target tests

popd