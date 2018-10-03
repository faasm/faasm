from os.path import dirname, realpath, join

PROJ_ROOT = dirname(dirname(realpath(__file__)))

TOOLCHAIN_DIR = join(PROJ_ROOT, "toolchain")
TOOLCHAIN_ROOT = join(TOOLCHAIN_DIR, "wasm-install")

WASM_LIB_DIR = join(PROJ_ROOT, "wasm", "lib")

SYSROOT = join(TOOLCHAIN_ROOT, "sysroot")

TARGET_TRIPLE = "wasm32-unknown-unknown-wasm"
CONFIG_TARGET = "wasm32"

COMPILER_FLAGS = [
    "--target={}".format(TARGET_TRIPLE),
    "--sysroot={}".format(SYSROOT),
]

COMPILER_FLAGS_STRING = " ".join(COMPILER_FLAGS)
COMPILER_FLAGS_STRING = "\"{}\"".format(COMPILER_FLAGS_STRING)

# Getting everything mapped properly here is crucial. If we end up
# using system defaults we can get some errors that are *very* hard
# to debug

CC = join(TOOLCHAIN_ROOT, "bin", "clang")
CPP = join(TOOLCHAIN_ROOT, "bin", "clang-cpp")
CFLAGS = COMPILER_FLAGS_STRING
CXX = join(TOOLCHAIN_ROOT, "bin", "clang++")
CXXFLAGS = COMPILER_FLAGS_STRING
LD = join(TOOLCHAIN_ROOT, "bin", "wasm-ld")
CROSS_COMPILE = join(TOOLCHAIN_ROOT, "llvm-")
AR = join(TOOLCHAIN_ROOT, "bin", "llvm-ar")
AS = join(TOOLCHAIN_ROOT, "bin", "llvm-as")
RANLIB = join(TOOLCHAIN_ROOT, "bin", "llvm-ranlib")

_ENV_TUPLES = [
    ("CC", CC),
    ("CPP", CPP),
    ("CFLAGS", CFLAGS),
    ("AR", AR),
    ("AS", AS),
    ("LD", LD),
    ("RANLIB", RANLIB),
    ("CROSS_COMPILE", CROSS_COMPILE),
    ("CXX", CXX),
    ("CXXFLAGS", CXXFLAGS),
]

ENV_DICT = {e[0]: e[1] for e in _ENV_TUPLES}
ENV_STR = " ".join(["{}={}".format(e[0], e[1]) for e in _ENV_TUPLES])
