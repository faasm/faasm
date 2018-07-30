#!/bin/bash

set -e

echo "Building  $1 / $2"

echo "Cleaning build dir"
rm -rf work
mkdir work

echo "Sourcing emsdk env"
source /usr/local/code/emsdk/emsdk_env.sh > /dev/null

echo "Building with emscripten"
pushd work
emcc ../func/function_$2.c -Os -g -s WASM=1 -o function.js
popd

echo "Recreating function in output dir"
rm -rf wasm/$1/$2
mkdir -p wasm/$1/$2
cp work/function.wasm work/function.wast wasm/$1/$2/
