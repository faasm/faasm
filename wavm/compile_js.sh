#/bin/bash

set -e

pushd work

emcc ../function.c -Os -s WASM=1

popd
