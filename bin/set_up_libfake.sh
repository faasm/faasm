#!/bin/bash

set -e

# Compile libfake
inv compile-libfake 

# Put it in place
cp /home/shillaker/faasm/toolchain/sysroot/lib/objects-Release/fake/src/fake.c.obj /usr/local/faasm/runtime_root/libfake.wasm

# Run codegen
./cmake-build-debug/bin/codegen /usr/local/faasm/runtime_root/libfake.wasm

# Generate wast
./cmake-build-debug/WAVM/bin/wavm-disas /usr/local/faasm/runtime_root/libfake.wasm /usr/local/faasm/runtime_root/libfake.wast
