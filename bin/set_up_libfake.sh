#!/bin/bash

set -e

EMSCRIPTEN=./pyodide/emsdk/emsdk/upstream/4778
SYSROOT=${EMSCRIPTEN}/sysroot

# Compile libfake
inv compile-libfake 

# Build the shared object, shared will force it to export everything
#${EMSCRIPTEN}/bin/wasm-ld \
#    --whole-archive ${SYSROOT}/lib/libfake.a \
#    --shared \
#    --import-memory \
#    --import-table \
#    -o /usr/local/faasm/runtime_root/libfake.so

# Copy shared object into place
cp ${SYSROOT}/lib/libfake.so /usr/local/faasm/runtime_root/

# Run codegen
./cmake-build-debug/bin/codegen /usr/local/faasm/runtime_root/libfake.so

# Generate wast
./cmake-build-debug/WAVM/bin/wavm-disas /usr/local/faasm/runtime_root/libfake.so /usr/local/faasm/runtime_root/libfake.wast
