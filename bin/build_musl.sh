#!/bin/bash

set -e

THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJ_ROOT=${THIS_DIR}/..
EM_ROOT=${PROJ_ROOT}/pyodide/emsdk/emsdk/upstream/latest
SYSROOT=${EM_ROOT}/sysroot
TOOL_BIN=${EM_ROOT}/bin

MUSL_DIR=${THIS_DIR}/../musl
MUSL_BUILD_DIR=/tmp/musl_out

CFLAGS="--sysroot=${SYSROOT} --target=wasm32-unknown-unknown"
CXXFLAGS="--sysroot=${SYSROOT} --target=wasm32-unknown-unknown"

CC="${TOOL_BIN}/wasm32-clang"
CPP="${TOOL_BIN}/clang-cpp"
CXX="${TOOL_BIN}/wasm32-clang++"
LD="${TOOL_BIN}/wasm-ld"
CROSS_COMPILE="${TOOL_BIN}/llvm-"
AR="${TOOL_BIN}/llvm-ar"
AS="${TOOL_BIN}/llvm-as"
RANLIB="${TOOL_BIN}/llvm-ranlib"

rm -rf ${MUSL_BUILD_DIR}
mkdir -p ${MUSL_BUILD_DIR}

echo "Building musl at ${MUSL_DIR} to ${MUSL_BUILD_DIR}"
BUILD_CMD="python2 ${MUSL_DIR}/libc.py --compile-to-wasm --clang_dir=${TOOL_BIN} --binaryen_dir="" --musl=${MUSL_DIR} --out=${MUSL_BUILD_DIR}/libc.a"

echo ""
echo "WARNING: this command may drop out the first time..."
echo "${BUILD_CMD}"
${BUILD_CMD}

# Create archive and put in place
echo "Creating archive"
pushd ${MUSL_BUILD_DIR}
${AR} rc libm.a
cp libc.a libm.a crt1.o ${SYSROOT}/lib/
popd

# Copy include files
echo "Copying include files"

# Default musl include files
cp -r ${MUSL_DIR}/include/* ${SYSROOT}/include/
cp -r ${MUSL_DIR}/arch/generic/bits/* ${SYSROOT}/include/bits/

# WASM-specific include files
cp -r ${MUSL_DIR}/arch/wasm32/bits/* ${SYSROOT}/include/bits/
cp -r ${MUSL_BUILD_DIR}/obj/include/bits/* ${SYSROOT}/include/bits/

# Imports file for linking 
cp ${MUSL_DIR}/arch/wasm32/libc.imports ${SYSROOT}/lib/

echo "Done"
