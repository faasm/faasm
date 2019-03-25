#!/bin/bash

set -e

TOOLCHAIN=$HOME/faasm/toolchain

# Compile libfake
inv compile-libfake 

# Build the shared object
ARCHIVE=${TOOLCHAIN}/sysroot/lib/libfake.a
${TOOLCHAIN}/bin/wasm-ld \
    --whole-archive ${TOOLCHAIN}/sysroot/lib/libfake.a \
    --shared \
    --import-memory \
    --import-table \
    -o /usr/local/faasm/runtime_root/libfake.so

# Run codegen
./cmake-build-debug/bin/codegen /usr/local/faasm/runtime_root/libfake.so

# Generate wast
./cmake-build-debug/WAVM/bin/wavm-disas /usr/local/faasm/runtime_root/libfake.so /usr/local/faasm/runtime_root/libfake.wast
