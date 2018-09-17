from os.path import dirname, realpath, join

PROJ_ROOT = dirname(dirname(realpath(__file__)))

LLVM_ROOT = join(PROJ_ROOT, "wasmception")

WASM_LIB_DIR = join(PROJ_ROOT, "wasm", "lib")

SYSROOT = join(LLVM_ROOT, "sysroot")

TARGET_TRIPLE="wasm32-unknown-unknown-wasm"

_FLAGS = [
    "--target={}".format(TARGET_TRIPLE),
    "--sysroot={}".format(SYSROOT),
    "-O3",
    "-fvisibility=hidden",
]
_FLAGS = ' '.join(_FLAGS)

_CLANG_BIN = join(LLVM_ROOT, "dist", "bin")
CC = join(_CLANG_BIN, "clang")
CFLAGS = _FLAGS
CXX = join(_CLANG_BIN, "clang++")
CXXFLAGS = _FLAGS
CROSS_COMPILE = join(LLVM_ROOT, "llvm-")

CPP = join(_CLANG_BIN, "clang-cpp")

ENV = {
    "CC": CC,
    "CFLAGS": CFLAGS,
    "CPP": CPP,
    "CXX": CXX,
    "CXXFLAGS": CXXFLAGS,
    "CROSS_COMPILE": CROSS_COMPILE,
}