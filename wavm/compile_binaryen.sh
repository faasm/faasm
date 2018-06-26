#!/bin/bash

set -e

pushd work > /dev/null

echo "Compiling to LLVM IR"
clang -S -emit-llvm ../function.c

echo "Running LLVM compiler to WASM"
/usr/local/code/llvm/build/bin/llc function.ll -mtriple=wasm32-unknown-unknown-elf -filetype=asm -o function.s

echo "Generating wasm text representation"
/usr/local/code/binaryen/bin/s2wasm function.s > function.wat

echo "Converting from wasm text to wasm binary"
/usr/local/code/binaryen/bin/wasm-as function.wat -o function.wasm

popd > /dev/null
