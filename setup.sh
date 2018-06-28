#!/bin/bash

set -e

echo "Setting up Faasm"

# Create tools directory
mkdir -p tools
pushd tools

# -------------------------
# LLVM and clang
# -------------------------

echo "Checking out LLVM"
git clone http://llvm.org/git/llvm.git

echo "Checking out clang"
pushd llvm
pushd tools
git clone http://llvm.org/git/clang.git
popd

echo "Checkout out compiler-rt"
pushd projects
git clone http://llvm.org/git/compiler-rt.git
popd

echo "Building LLVM (takes ages...)"
mkdir -p build
pushd build
cmake -G Ninja -DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD=WebAssembly -DLLVM_TARGETS_TO_BUILD= -DCMAKE_BUILD_TYPE=Release ..

ninja
popd

popd

# -----------------------
# Binaryen
# -----------------------

echo "Checking out binaryen"
git clone https://github.com/Webassembly/binaryen
pushd binaryen
cmake . && make
popd

# -----------------------
# WAVM
# -----------------------
echo "Checking out WAVM (you'll need to build this through CLion)"
git clone https://github.com/AndrewScheidecker/WAVM

# -----------------------
# WABT
# -----------------------
echo "Checking out wabt"
git clone --recursive https://github.com/WebAssembly/wabt
pushd wabt
make
popd

# ----------------------
# musl
# ----------------------
echo "Checking out musl"
git clone git@github.com:Shillaker/musl.git


popd
