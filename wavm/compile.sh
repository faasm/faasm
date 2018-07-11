#!/bin/bash

set -e

echo "Cleaning build dir"
rm -rf work/*

echo "Sourcing emsdk env"
source /usr/local/code/emsdk/emsdk_env.sh > /dev/null

echo "Building with emscripten"
pushd work > /dev/null

# Options
# -s USE_PTHREADS=1
emcc ../func/function_$1.c -Os -g -s WASM=1 -o function.js

popd > /dev/null
