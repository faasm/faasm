#/bin/bash

set -e

pushd work > /dev/null

/usr/local/code/WAVM/cmake-build-release/bin/wavm --disable-emscripten function.wasm

popd > /dev/null
