#!/bin/bash
set -e
set -x

CODE_DIR=/usr/local/code/
if [[ ! -d ${CODE_DIR} ]]; then
  echo "CODE_DIR (${CODE_DIR}) does not exists"
  exit 1
fi

LLVM_DIR=${CODE_DIR}/llvm-perf
BUILD_DIR=${LLVM_DIR}/build

# Clone LLVM-9 repo if not done already. Version must be compatible with WAVM
if [[ ! -d "$LLVM_DIR" ]]; then
    git clone https://github.com/llvm/llvm-project ${LLVM_DIR}
    pushd ${LLVM_DIR}
    git checkout llvmorg-9.0.0-rc3
    popd
fi

# Make the build dir
mkdir -p ${BUILD_DIR}
pushd ${BUILD_DIR}

# Clear build dir
if [[ -f "${BUILD_DIR}/build.ninja" ]]; then
  ninja clean
fi

# Enable JIT profiling features of LLVM
cmake \
	-G Ninja \
	-DCMAKE_C_COMPILER=/usr/bin/clang \
	-DCMAKE_CXX_COMPILER=/usr/bin/clang++ \
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
