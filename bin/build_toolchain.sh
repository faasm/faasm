#!/bin/bash

# Last known good revisions taken from https://storage.googleapis.com/wasm-llvm/builds/linux/lkgr.json
# There is a parameter --sync-lkgr on the command, but it appears not to work

FAASM_HOME=${HOME}/faasm
TOOLCHAIN=${FAASM_HOME}/toolchain

set -e

LLVM_HASH=c60bc94afcf77fa4487cec52ab5323a1f5469f5b
MUSL_HASH=16d3d3825b4bd125244e43826fb0f0da79a1a4ad
BINARYEN_HASH=dd3873375ca7375b79105742b30fa03f9f8a0c24

# Check out code
pushd waterfall/src
python build.py --no-build --sync-include binaryen,cmake,llvm,host-toolchain,cr-buildtools,musl --no-tool-tests --no-test

pushd llvm-project
git checkout ${LLVM_HASH}
popd

pushd musl
git checkout ${MUSL_HASH}
popd

pushd binaryen
git checkout ${BINARYEN_HASH}
popd

# Run the build
python build.py --build-include llvm,binaryen,musl,compiler-rt,libcxx,libcxxabi --no-sync --no-tool-tests --no-test

# Put everything in place
rm -rf ${TOOLCHAIN}
cp -r work/wasm-install ${TOOLCHAIN}

# Check it works
${TOOLCHAIN}/bin/clang --version

popd
