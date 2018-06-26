#/bin/bash

set -e

pushd work > /dev/null

/usr/local/code/WAVM/cmake-build-release/bin/wavm a.out.wasm

popd > /dev/null
