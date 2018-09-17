from os.path import dirname, realpath, join

PROJ_ROOT = dirname(dirname(realpath(__file__)))

LLVM_ROOT = join(PROJ_ROOT, "wasmception")

WASM_LIB_DIR = join(PROJ_ROOT, "wasm", "lib")

SYSROOT = join(LLVM_ROOT, "sysroot")

TARGET_TRIPLE = "wasm32-unknown-unknown-wasm"
CONFIG_TARGET = "wasm32"

COMPILER_FLAGS = [
    "--target={}".format(TARGET_TRIPLE),
    "--sysroot={}".format(SYSROOT),
]

COMPILER_FLAGS_STRING = " ".join(COMPILER_FLAGS)
COMPILER_FLAGS_STRING = "\"{}\"".format(COMPILER_FLAGS_STRING)

_CLANG_BIN = join(LLVM_ROOT, "dist", "bin")
CC = join(_CLANG_BIN, "clang")
CFLAGS = COMPILER_FLAGS_STRING
CXX = join(_CLANG_BIN, "clang++")
CXXFLAGS = COMPILER_FLAGS_STRING
CROSS_COMPILE = join(LLVM_ROOT, "llvm-")

CPP = join(_CLANG_BIN, "clang-cpp")

_ENV_TUPLES = [
    ("CC", CC),
    ("CFLAGS", CFLAGS),
    ("CPP", CPP),
    ("CXX", CXX),
    ("CXXFLAGS", CXXFLAGS),
    ("CROSS_COMPILE", CROSS_COMPILE),
]

ENV_DICT = {e[0]: e[1] for e in _ENV_TUPLES}
ENV_STR = " ".join(["{}={}".format(e[0], e[1]) for e in _ENV_TUPLES])
