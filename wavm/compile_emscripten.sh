#/bin/bash

set -e

echo "Sourcing emsdk env"
source /usr/local/code/emsdk/emsdk_env.sh > /dev/null

pushd work > /dev/null

echo "Building with emcc"
emcc ../function.c -Os -g -s WASM=1

popd > /dev/null
