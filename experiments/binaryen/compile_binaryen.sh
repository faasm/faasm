#!/bin/bash

set -e

pushd work > /dev/null

echo "Compiling to LLVM IR"
/usr/local/code/faasm/tools/llvm/build/bin/clang -emit-llvm --target=wasm32-unknown-unknown-elf -nostdlib -S ../function.c

echo "Running LLVM compiler to WASM"
/usr/local/code/faasm/tools/llvm/build/bin/llc -march=wasm32 -o function.s function.ll

echo "Generating wasm text representation"
/usr/local/code/faasm/tools/binaryen/bin/s2wasm -o function.wast function.s

echo "Converting from wast to wasm"
/usr/local/code/faasm/tools/binaryen/bin/wasm-as -o function.wasm function.wast 

popd > /dev/null
