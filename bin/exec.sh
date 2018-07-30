#!/bin/bash

set -e

pushd work

../cmake-build-release/WAVM/bin/wavm function.wasm

popd