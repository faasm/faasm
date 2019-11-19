#!/bin/bash

set -e

pushd third-party/gem3-mapper

export CXX=/usr/bin/clang++
export CC=/usr/bin/clang
export CPP=/usr/bin/clang-cpp
export LINK=/usr/bin/clang++

./configure --enable-cuda=no

# Release build
make

# Debug build
# make debug

popd
