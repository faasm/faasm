from os.path import join

WASM_SYSROOT = "/usr/local/faasm/llvm-sysroot"
WASM_TOOLCHAIN_BIN = "/usr/local/faasm/toolchain/bin"

WASM_CC = join(WASM_TOOLCHAIN_BIN, "clang")
WASM_CXX = join(WASM_TOOLCHAIN_BIN, "clang++")
WASM_CPP = join(WASM_TOOLCHAIN_BIN, "clang-cpp")
WASM_AR = join(WASM_TOOLCHAIN_BIN, "llvm-ar")
WASM_NM = join(WASM_TOOLCHAIN_BIN, "llvm.nm")
WASM_RANLIB = join(WASM_TOOLCHAIN_BIN, "llvm-ranlib")
WASM_LD = join(WASM_TOOLCHAIN_BIN, "wasm-ld")
WASM_LDSHARED = join(WASM_TOOLCHAIN_BIN, "wasm-ld")

WASM_LDFLAGS = "-Xlinker --stack-first -Xlinker --no-check-features"

WASM_BUILD = "wasm32"
WASM_HOST = "wasm32-unknown-none"
WASM_HOST_UNKNOWN = "wasm32-unknown-unknown"

WASM_CFLAGS = "-O3 --sysroot={} -msimd128".format(WASM_SYSROOT)
WASM_CXXFLAGS = "-O3 --sysroot={} -msimd128".format(WASM_SYSROOT)

BASE_CONFIG_CMD = [
    "CC={}".format(WASM_CC),
    "CXX={}".format(WASM_CXX),
    "CPP={}".format(WASM_CPP),
    "AR={}".format(WASM_AR),
    "RANLIB={}".format(WASM_RANLIB),
]

BASE_CONFIG_FLAGS = [
    "CFLAGS=\"{} -ldlmalloc\"".format(WASM_CFLAGS),
    "CXXFLAGS=\"{} -ldlmalloc\"".format(WASM_CXXFLAGS),
    "LDFLAGS=\"{}\"".format(WASM_LDFLAGS),
]
