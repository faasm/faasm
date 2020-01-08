#!/bin/bash

set -e

source toolchain/native_clang.sh

pushd third-party/gem3-mapper

./configure --enable-cuda=no

# Release build
make -j

# Debug build
# make debug

popd
