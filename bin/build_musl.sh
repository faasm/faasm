#!/bin/bash

# set -e

FAASM_HOME=${HOME}/faasm
SYSROOT=${FAASM_HOME}/sysroot
MUSL_DIR=${FAASM_HOME}/musl
MUSL_BUILD_DIR=${FAASM_HOME}/musl_out

source bin/wasm_env.sh

if [ -d ${MUSL_DIR} ]; then
   echo "Not cloning musl, already exists"
else
    echo "Cloning musl repo"
    pushd ${FAASM_HOME}
    git clone https://github.com/Shillaker/musl.git
    popd
fi

rm -rf ${MUSL_BUILD_DIR}
mkdir -p ${MUSL_BUILD_DIR}

echo "Building musl at ${MUSL_DIR} to ${MUSL_BUILD_DIR}"
exec python2 ${MUSL_DIR}/libc.py \
    --clang_dir=/usr/bin \
    --binaryen_dir="" \
    --musl=${MUSL_DIR} \
    --out=${MUSL_BUILD_DIR}/libc.a \
    --compile-to-wasm

# Set up dirs
echo "Setting up sysroot"
mkdir -p ${SYSROOT}/lib
mkdir -p ${SYSROOT}/include
mkdir -p ${SYSROOT}/include/bits

# Create archive and put in place
echo "Creating archive"
pushd ${MUSL_BUILD_DIR}
llvm-ar rc libm.a
cp libc.a libm.a crt1.o ${SYSROOT}/lib
popd

# Copy include files
echo "Copying include files"
pushd ${MUSL_DIR}
cp arch/wasm32/libc.imports ${SYSROOT}/lib
cp -r include ${SYSROOT}/
cp -r arch/generic/bits/* arch/wasm32/bits* ${SYSROOT}/include/bits
popd