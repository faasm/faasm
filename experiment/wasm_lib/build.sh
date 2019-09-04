#!/bin/bash

set -e

rm -rf build
mkdir build
pushd build

cmake \
  -DCMAKE_TOOLCHAIN_FILE=/usr/local/code/faasm/toolchain/FaasmToolchain.cmake \
  -DCMAKE_BUILD_TYPE=Debug \
  ..

make