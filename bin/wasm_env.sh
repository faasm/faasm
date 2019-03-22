#!/bin/bash

set -e

echo "Setting up toolchain for wasm"

export FAASM_HOME=${HOME}/faasm
export SYSROOT=${FAASM_HOME}/sysroot
mkdir -p ${SYSROOT}

export CFLAGS="--sysroot=${SYSROOT} --target=wasm32"
export CXXFLAGS="--sysroot=${SYSROOT} --target=wasm32"

export CC="/usr/bin/clang-8"
export CPP="/usr/bin/clang-cpp-8"
export CXX="/usr/bin/clang++-8"
export LD="/usr/bin/wasm-ld-8"
export CROSS_COMPILE="/usr/bin/llvm-"
export AR="/usr/bin/llvm-ar-8"
export AS="/usr/bin/llvm-as-8"
export RANLIB="/usr/bin/llvm-ranlib-8"
