#!/bin/bash

set -e

pushd work > /dev/null

echo "Compiling to LLVM IR"
clang -S -emit-llvm ../function.c

echo "Running LLVM compiler to WASM"
/usr/local/code/llvm/bin/llc function.ll -mtriple=wasm32-unknown-unknown-elf -filetype=asm -o function.s

echo "Generating wasm text representation"
/usr/local/code/binaryen/bin/s2wasm code.s > code.wat

echo "Converting from wasm text to wasm binary"
/usr/local/code/wabt/out/clang/Debug/wat2wasm code.wat

popd > /dev/null
