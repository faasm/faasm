#!/bin/bash

set -e

THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

if [ "$1" == "llvm" ]; then
  SYSROOT=/usr/local/faasm/llvm-sysroot
  CLANG_DIR=/usr/bin
else
  # Normal emsdk stuff
  EM_ROOT=/usr/local/faasm/emsdk/upstream/latest
  SYSROOT=${EM_ROOT}/sysroot
  CLANG_DIR=${EM_ROOT}/bin

fi

mkdir -p ${SYSROOT}

TARGET="wasm32-unknown-unknown"
CC="${CLANG_DIR}/clang"
CXX="${CLANG_DIR}/clang++"
CROSS_COMPILE="${CLANG_DIR}/llvm-"
CPP="${CLANG_DIR}/clang-cpp"
LD="${CLANG_DIR}/wasm-ld"
AR="${CLANG_DIR}/llvm-ar"
AS="${CLANG_DIR}/llvm-as"
RANLIB="${CLANG_DIR}/llvm-ranlib"

MUSL_DIR=${THIS_DIR}/../musl
MUSL_BUILD_DIR=/tmp/musl_out

CFLAGS="--sysroot=${SYSROOT} --target=${TARGET}"
CXXFLAGS="--sysroot=${SYSROOT} --target=${TARGET}"

rm -rf ${MUSL_BUILD_DIR}
mkdir -p ${MUSL_BUILD_DIR}

echo "Building musl at ${MUSL_DIR} to ${MUSL_BUILD_DIR}"
BUILD_CMD="python2 ${MUSL_DIR}/libc.py --compile-to-wasm --clang_dir=${CLANG_DIR} --binaryen_dir="" --musl=${MUSL_DIR} --out=${MUSL_BUILD_DIR}/libc.a"

echo ""
echo "WARNING: this command may drop out the first time..."
echo "${BUILD_CMD}"
${BUILD_CMD}

mkdir -p ${SYSROOT}/lib
mkdir -p ${SYSROOT}/include/bits

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
