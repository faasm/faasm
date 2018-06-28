#/bin/bash

set -e

echo "Sourcing emsdk env"
source /usr/local/code/emsdk/emsdk_env.sh > /dev/null

pushd work > /dev/null

# -g causes wast output
echo "Building with emcc"
emcc ../function.c -Os -s WASM=1

popd > /dev/null
