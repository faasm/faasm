#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
TOOLCHAIN=${THIS_DIR}/../../toolchain
TC_FILE=${TOOLCHAIN}/FaasmToolchain.cmake

rm -rf build
mkdir build
pushd build

cmake -DCMAKE_TOOLCHAIN_FILE=${TC_FILE} \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  ..

make

popd
