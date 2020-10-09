from copy import copy
from os.path import join
from subprocess import run

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

# Use top-level clang as the linker rather than invoking wasm-ld directly
WASM_LD = WASM_CC
WASM_LDXX = WASM_CXX

# Host triple
WASM_BUILD = "wasm32"
WASM_HOST = "wasm32-unknown-wasi"
WASM_HOST_SHARED = "wasm32-unknown-emscripten"
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
    "--target={}".format(WASM_HOST_SHARED),
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

WASM_CCSHARED = " ".join([WASM_CC] + WASM_CFLAGS_SHARED)
WASM_CXXSHARED = " ".join([WASM_CXX] + WASM_CFLAGS_SHARED)
WASM_LDSHARED = " ".join([WASM_CC] + WASM_LDFLAGS_SHARED)
WASM_LDXXSHARED = " ".join([WASM_CXX] + WASM_LDFLAGS_SHARED)

_BASE_CONFIG_CMD = [
    "CC={}".format(WASM_CC),
    "CXX={}".format(WASM_CXX),
    "CPP={}".format(WASM_CPP),
    "AR={}".format(WASM_AR),
    "RANLIB={}".format(WASM_RANLIB),
    'CFLAGS="{}"'.format(" ".join(WASM_CFLAGS)),
    'CPPFLAGS="{}"'.format(" ".join(WASM_CFLAGS)),
    'CXXFLAGS="{}"'.format(" ".join(WASM_CXXFLAGS)),
    'CCSHARED="{}"'.format(WASM_CCSHARED),
    'CXXSHARED="{}"'.format(WASM_CXXSHARED),
]

_BASE_CONFIG_ARGS = [
    "--build={}".format(WASM_BUILD),
    "--host={}".format(WASM_HOST),
]

_BASE_CONFIG_ARGS_SHARED = [
    "--build={}".format(WASM_BUILD),
    "--host={}".format(WASM_HOST_SHARED),
]

BASE_CONFIG_CMD = _BASE_CONFIG_CMD + [
    "LD={}".format(WASM_LD),
    'LDSHARED="{}"'.format(WASM_LDSHARED),
]

BASE_CONFIG_CMDXX = _BASE_CONFIG_CMD + [
    "LD={}".format(WASM_LDXX),
    'LDSHARED="{}"'.format(WASM_LDXXSHARED),
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


def build_config_cmd(cmd, shared=False, cxx=False, conf_args=True):
    """
    Wraps an autotools command in the relevant environment variables and
    cross-compilation config
    """
    result = copy(BASE_CONFIG_CMDXX if cxx else BASE_CONFIG_CMD)

    result += BASE_CONFIG_FLAGS_SHARED if shared else BASE_CONFIG_FLAGS

    result += cmd

    if conf_args:
        result += _BASE_CONFIG_ARGS_SHARED if shared else _BASE_CONFIG_ARGS

    return result


def run_autotools(proj_dir):
    """
    Runs through autotools set-up on the given directory
    """

    def _run_auto_cmd(cmd):
        print("Running {}".format(cmd))
        auto_cmd = build_config_cmd([cmd], conf_args=False)
        auto_cmd = " ".join(auto_cmd)
        run(auto_cmd, shell=True, check=True, cwd=proj_dir)

    _run_auto_cmd("libtoolize")
    _run_auto_cmd("aclocal")
    _run_auto_cmd("autoheader")
    _run_auto_cmd("autoconf")
    _run_auto_cmd("automake --add-missing")
