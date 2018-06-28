#/bin/bash

set -e

pushd work > /dev/null

# Can switch off emscripten support with --disable-emscripten
/usr/local/code/faasm/tools/WAVM/cmake-build-release/bin/wavm function.wasm musl.wasm

popd > /dev/null
