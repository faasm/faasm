from os.path import join

# Directories
WASM_SYSROOT = "/usr/local/faasm/llvm-sysroot"
WASM_LIB_INSTALL = "{}/lib/wasm32-wasi".format(WASM_SYSROOT)
WASM_TOOLCHAIN_BIN = "/usr/local/faasm/toolchain/bin"

# Executables
WASM_CC = join(WASM_TOOLCHAIN_BIN, "clang")
WASM_CXX = join(WASM_TOOLCHAIN_BIN, "clang++")
WASM_CPP = join(WASM_TOOLCHAIN_BIN, "clang-cpp")
WASM_AR = join(WASM_TOOLCHAIN_BIN, "llvm-ar")
WASM_NM = join(WASM_TOOLCHAIN_BIN, "llvm.nm")
WASM_RANLIB = join(WASM_TOOLCHAIN_BIN, "llvm-ranlib")
WASM_LD = join(WASM_TOOLCHAIN_BIN, "wasm-ld")

# Host triple
WASM_BUILD = "wasm32"
WASM_HOST = "wasm32-unknown-wasi"
WASM_HOST_UNKNOWN = "wasm32-unknown-unknown"

# CFLAGS
WASM_CFLAGS = [
    "-m32",
    "-DCONFIG_32",
    "-DANSI",
    "-O3",
    "--sysroot={}".format(WASM_SYSROOT),
    "-D__faasm",
]

WASM_CXXFLAGS = WASM_CFLAGS

WASM_CFLAGS_SHARED = [
    "-D__wasi__",
    "-nostdlib",
    "-nostdlib++",
    "-fPIC",
    "--target=wasm32-unknown-emscripten",
]
WASM_CXXFLAGS_SHARED = WASM_CFLAGS_SHARED

# LDFLAGS
WASM_LDFLAGS = [
    "-static",
    "-Xlinker --no-gc-sections",
    "-Xlinker --stack-first",
    "-Xlinker --no-check-features",
]

WASM_LDFLAGS_SHARED = [
    "-nostdlib",
    "-nostdlib++",
    "-Xlinker --no-entry",
    "-Xlinker --shared",
    "-Xlinker --export-all",
    "-Xlinker --no-gc-sections",
]

BASE_CONFIG_CMD = [
    "CC={}".format(WASM_CC),
    "CXX={}".format(WASM_CXX),
    "CPP={}".format(WASM_CPP),
    "AR={}".format(WASM_AR),
    "RANLIB={}".format(WASM_RANLIB),
    'CFLAGS="{}"'.format(" ".join(WASM_CFLAGS)),
    'CPPFLAGS="{}"'.format(" ".join(WASM_CFLAGS)),
    'CXXFLAGS="{}"'.format(WASM_CXXFLAGS),
]

BASE_CONFIG_FLAGS = [
    'CFLAGS="{}"'.format(" ".join(WASM_CFLAGS)),
    'CPPFLAGS="{}"'.format(" ".join(WASM_CXXFLAGS)),
    'LDFLAGS="{}"'.format(" ".join(WASM_LDFLAGS)),
]

BASE_CONFIG_FLAGS_SHARED = [
    'CFLAGS="{}"'.format(" ".join(WASM_CFLAGS_SHARED)),
    'CPPFLAGS="{}"'.format(" ".join(WASM_CXXFLAGS_SHARED)),
    'LDFLAGS="{}"'.format(" ".join(WASM_LDFLAGS_SHARED)),
]
