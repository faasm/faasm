#!/bin/bash

set -e

SYSROOT=/usr/local/faasm/emsdk/upstream/latest/sysroot
RUNTIME_ROOT=/usr/local/faasm/runtime_root/

# Compile libfake
inv compile-libfake --clean

# Copy shared object into place
cp ${SYSROOT}/lib/libfake*.so ${RUNTIME_ROOT}

# Run codegen
./cmake-build-debug/bin/codegen ${RUNTIME_ROOT}libfakeLibA.so
./cmake-build-debug/bin/codegen ${RUNTIME_ROOT}libfakeLibB.so

# Generate wast
./cmake-build-debug/WAVM/bin/wavm-disas ${RUNTIME_ROOT}libfakeLibA.so ${RUNTIME_ROOT}libfakeLibA.wast
./cmake-build-debug/WAVM/bin/wavm-disas ${RUNTIME_ROOT}libfakeLibB.so ${RUNTIME_ROOT}libfakeLibB.wast
