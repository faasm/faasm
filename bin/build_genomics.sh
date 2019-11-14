#!/bin/bash

set -e

source toolchain/env.sh

export CFLAGS="--sysroot=${WASM_SYSROOT} ${WASM_LDFLAGS}"

export CPPFLAGS="${CFLAGS} -DHAVE_UNSETENV=1 -DHAVE_PUTENV=1 -DHAVE_TIMEGM=1 -DHAVE_FORK=1"

export CC="${WASM_CC} ${CFLAGS}"
export CXX="${WASM_CXX} ${CPPFLAGS}"
export AR=${WASM_AR}
export RANLIB=${WASM_RANLIB}
export SYSROOT=${WASM_SYSROOT}
export LDSHARED="${WASM_LDSHARED}"

export WASM_BUILD=1

pushd third-party/gem3-mapper

./configure --enable-cuda=no --disable-lto --without-tests --host=${WASM_HOST} --build=${WASM_BUILD}

make

popd
