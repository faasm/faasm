#!/bin/bash

set -e

FAASM_HOME=${HOME}/faasm
TOOL_BIN=${FAASM_HOME}/toolchain/bin
SYSROOT=${FAASM_HOME}/toolchain/sysroot
MUSL_DIR=${FAASM_HOME}/musl
MUSL_BUILD_DIR=${FAASM_HOME}/musl_out

CFLAGS="--sysroot=${SYSROOT} --target=wasm32-unknown-unknown"
CXXFLAGS="--sysroot=${SYSROOT} --target=wasm32-unknown-unknown"

CC="${TOOL_BIN}/clang"
CPP="${TOOL_BIN}/clang-cpp"
CXX="${TOOL_BIN}/clang++"
LD="${TOOL_BIN}/wasm-ld"
CROSS_COMPILE="${TOOL_BIN}/llvm-"
AR="${TOOL_BIN}/llvm-ar"
AS="${TOOL_BIN}/llvm-as"
RANLIB="${TOOL_BIN}/llvm-ranlib"

if [ -d ${MUSL_DIR} ]; then
   echo "Not cloning musl, updating"
    pushd ${MUSL_DIR}
    git pull
    popd
else
    echo "Cloning musl repo"
    pushd ${FAASM_HOME}
    git clone https://github.com/Shillaker/musl.git
    popd
fi

rm -rf ${MUSL_BUILD_DIR}
mkdir -p ${MUSL_BUILD_DIR}

echo "Building musl at ${MUSL_DIR} to ${MUSL_BUILD_DIR}"
BUILD_CMD="python2 ${MUSL_DIR}/libc.py --compile-to-wasm --clang_dir=${TOOL_BIN} --binaryen_dir="" --musl=${MUSL_DIR} --out=${MUSL_BUILD_DIR}/libc.a"

echo ""
echo "WARNING: this command may drop out the first time..."
echo "${BUILD_CMD}"
${BUILD_CMD}

# Set up dirs
echo "Setting up sysroot"
mkdir -p ${SYSROOT}/lib
mkdir -p ${SYSROOT}/include
mkdir -p ${SYSROOT}/include/bits

# Create archive and put in place
echo "Creating archive"
pushd ${MUSL_BUILD_DIR}
llvm-ar rc libm.a
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