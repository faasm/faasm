#/bin/bash

set -e

pushd work > /dev/null

# Can switch off emscripten support with --disable-emscripten
/usr/local/code/WAVM/cmake-build-release/bin/wavm function.wasm

popd > /dev/null
