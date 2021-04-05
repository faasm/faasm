#!/bin/bash
set -e
set -x

# See https://github.com/llvm/llvm-project/releases
# NOTE - upgrading this tag may break the WAVM integration, be prepared to fix 
# 31/03/21 - LLVM 9.0.1 definitely works 
LLVM_TAG=llvmorg-9.0.1

THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJ_ROOT=${THIS_DIR}/..

# Set up directory
DEV_DIR=${PROJ_ROOT}/dev
mkdir -p ${DEV_DIR}
LLVM_DIR=${PROJ_ROOT}/dev/llvm-perf

# Clone LLVM repo if not done already
if [[ ! -d "$LLVM_DIR" ]]; then
    git clone \
        --branch ${LLVM_TAG} \
        --depth 1 \
        https://github.com/llvm/llvm-project \
        ${LLVM_DIR}
fi

# Make the build dir
BUILD_DIR=${LLVM_DIR}/build
mkdir -p ${BUILD_DIR}
pushd ${BUILD_DIR}

# Build LLVM with JIT profiling turned on
# See https://llvm.org/docs/CMake.html
cmake \
	-G Ninja \
	-DCMAKE_C_COMPILER=/usr/bin/clang-10 \
	-DCMAKE_CXX_COMPILER=/usr/bin/clang++-10 \
	-DCMAKE_BUILD_TYPE=RelWithDebInfo \
	-DLLVM_USE_PERF=1 \
	-DLLVM_USE_INTEL_JITEVENTS=1 \
	-DLLVM_TARGETS_TO_BUILD=X86 \
	-DLLVM_INCLUDE_TOOLS=0 \
	-DLLVM_INCLUDE_TESTS=0 \
	-DLLVM_INCLUDE_EXAMPLES=0 \
	-DLLVM_INCLUDE_BENCHMARKS=0 \
	../llvm

ninja

popd
