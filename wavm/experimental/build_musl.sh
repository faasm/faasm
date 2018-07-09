#!/bin/bash

set -e

pushd ../tools/musl

echo "Compiling musl"
python libc.py --clang_dir /usr/local/code/faasm/tools/llvm/build/bin --binaryen_dir /usr/local/code/faasm/tools/binaryen/bin --musl /usr/local/code/faasm/tools/musl --sexpr_wasm /usr/local/code/faasm/tools/wabt/bin/wat2wasm --out /usr/local/code/faasm/wavm/work/musl.wast

popd > /dev/null

pushd work > /dev/null

echo "Converting from wast to wasm"
/usr/local/code/faasm/tools/binaryen/bin/wasm-as -o musl.wasm musl.wast 

popd > /dev/null

