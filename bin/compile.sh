#!/bin/bash

set -e

echo "Building  $1 / $2"

echo "Cleaning build dir"
rm -rf work
mkdir work

echo "Sourcing emsdk env"
source /usr/local/code/emsdk/emsdk_env.sh > /dev/null

echo "Building with emscripten"
# NOTE: can include -g here to generate .wast
pushd work
emcc ../func/function_$2.c -g -Oz -s WASM=1 -o function.js -I ../include/faasm
popd

echo "Recreating function in output dir"
rm -rf wasm/$1/$2
mkdir -p wasm/$1/$2
cp work/function.* wasm/$1/$2/
