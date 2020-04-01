export WASM_SYSROOT=/usr/local/faasm/llvm-sysroot
export WASM_TOOLCHAIN_BIN=/usr/local/faasm/toolchain/bin

export WASM_CC=${WASM_TOOLCHAIN_BIN}/clang
export WASM_CXX=${WASM_TOOLCHAIN_BIN}/clang++
export WASM_CPP=${WASM_TOOLCHAIN_BIN}/clang-cpp
export WASM_AR=${WASM_TOOLCHAIN_BIN}/llvm-ar
export WASM_NM=${WASM_TOOLCHAIN_BIN}/llvm-nm
export WASM_RANLIB=${WASM_TOOLCHAIN_BIN}/llvm-ranlib
export WASM_LD=${WASM_TOOLCHAIN_BIN}/wasm-ld
export WASM_LDSHARED=${WASM_TOOLCHAIN_BIN}/wasm-ld

export WASM_LDFLAGS="-Xlinker --stack-first -Xlinker --no-check-features"

export WASM_BUILD=wasm32
export WASM_HOST=wasm32-unknown-wasi
export WASM_HOST_UNKNOWN=wasm32-unknown-unknown

export WASM_CFLAGS="-O3 --sysroot=${WASM_SYSROOT} -msimd128 -mno-atomics -D__faasm"
export WASM_CXXFLAGS=${WASM_CFLAGS}

