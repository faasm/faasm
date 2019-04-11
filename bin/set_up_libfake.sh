#!/bin/bash

set -e

SYSROOT=./pyodide/emsdk/emsdk/upstream/latest/sysroot

# Compile libfake
inv compile-libfake --clean

# Copy shared object into place
cp ${SYSROOT}/lib/libfake.so /usr/local/faasm/runtime_root/

# Run codegen
./cmake-build-debug/bin/codegen /usr/local/faasm/runtime_root/libfake.so

# Generate wast
./cmake-build-debug/WAVM/bin/wavm-disas /usr/local/faasm/runtime_root/libfake.so /usr/local/faasm/runtime_root/libfake.wast
