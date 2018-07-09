#!/bin/bash

set -e

pushd work > /dev/null

# Can switch off emscripten support with --disable-emscripten
# Note, using CLion-built version here
/usr/local/code/WAVM/cmake-build-debug/bin/wavm function.wasm --trace-syscalls

popd > /dev/null
