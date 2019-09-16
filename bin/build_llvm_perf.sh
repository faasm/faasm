#!/bin/bash
set -e

FAASM_DIR=${HOME}/faasm
LLVM_DIR=${FAASM_DIR}/llvm
BUILD_DIR=${LLVM_DIR}/build

# Clone LLVM repo if not done already
mkdir -p ${FAASM_DIR}
if [[ ! -d "$LLVM_DIR" ]]; then
    git clone https://github.com/llvm/llvm-project ${LLVM_DIR}
    pushd ${LLVM_DIR}
    git checkout llvmorg-9.0.0-rc3
    popd
fi

# Clear build dir
# rm -rf ${BUILD_DIR}

# Make the build dir if it exists
mkdir -p ${BUILD_DIR}

pushd ${BUILD_DIR}

# Enable JIT profiling features of LLVM
cmake \
	-G Ninja \
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
