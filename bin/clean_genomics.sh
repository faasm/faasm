#!/bin/bash

set -e

pushd third-party/gem3-mapper

make clean

export WASM_BUILD=1
make clean

popd