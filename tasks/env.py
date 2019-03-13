import os
from os import makedirs
from os.path import dirname, realpath, join, exists, expanduser
from subprocess import call

# -------------------------------------------
# Misc
# -------------------------------------------

HOME_DIR = expanduser("~")
PROJ_ROOT = dirname(dirname(realpath(__file__)))

FAASM_HOME = join(HOME_DIR, "faasm")
FAASM_STORAGE = "/usr/local/faasm"
FAASM_RUNTIME_ROOT = join(FAASM_STORAGE, "runtime_root")

ANSIBLE_DIR = join(PROJ_ROOT, "ansible")

WASM_FUNC_BUILD_DIR = join(PROJ_ROOT, "func", "wasm_func_build")
EMSCRIPTEN_FUNC_BUILD_DIR = join(PROJ_ROOT, "func", "emscripten_func_build")

WASM_DIR = join(PROJ_ROOT, "wasm")

RUNTIME_S3_BUCKET = "faasm-runtime"
STATE_S3_BUCKET = "faasm-state"

# TODO - avoid hard-coding
AWS_ACCOUNT_ID = "733781933474"
AWS_REGION = "eu-west-1"

# ---------------------------------------------
# Compile related
# ---------------------------------------------

# Note, most of the time this will be run inside the toolchain container
TOOLCHAIN_ROOT = "/toolchain"

WASM_LIB_DIR = join(PROJ_ROOT, "wasm", "lib")

SYSROOT = join(TOOLCHAIN_ROOT, "sysroot")

TARGET_TRIPLE = "wasm32-unknown-unknown-wasm"
CONFIG_TARGET = "wasm32"
CONFIG_HOST = "wasm32-unknown-none"

COMPILER_FLAGS = [
    "--target={}".format(TARGET_TRIPLE),
    "--sysroot={}".format(SYSROOT),
]

COMPILER_FLAGS_STRING = " ".join(COMPILER_FLAGS)
COMPILER_FLAGS_STRING = "\"{}\"".format(COMPILER_FLAGS_STRING)

# Getting everything mapped properly here is crucial. If we end up
# using system defaults we can get some errors that are *very* hard
# to debug

NATIVE_CC = "/usr/bin/clang"
NATIVE_CPP = "/usr/bin/clang-cpp"
NATIVE_CFLAGS = ""
NATIVE_CXX = "/usr/bin/clang++"
NATIVE_CXXFLAGS = ""
NATIVE_LD = "/usr/bin/ld"
NATIVE_CROSS_COMPILE = ""
NATIVE_AR = "/usr/bin/ar"
NATIVE_AS = "/usr/bin/as"
NATIVE_RANLIB = "/usr/bin/ranlib"

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

_NATIVE_ENV_TUPLES = [
    ("CC", NATIVE_CC),
    ("CPP", NATIVE_CPP),
    ("CFLAGS", NATIVE_CFLAGS),
    ("AR", NATIVE_AR),
    ("AS", NATIVE_AS),
    ("LD", NATIVE_LD),
    ("RANLIB", NATIVE_RANLIB),
    ("CROSS_COMPILE", NATIVE_CROSS_COMPILE),
    ("CXX", NATIVE_CXX),
    ("CXXFLAGS", NATIVE_CXXFLAGS),
]

ENV_DICT = {e[0]: e[1] for e in _ENV_TUPLES}
ENV_STR = " ".join(["{}={}".format(e[0], e[1]) for e in _ENV_TUPLES])

NATIVE_ENV_DICT = {e[0]: e[1] for e in _NATIVE_ENV_TUPLES}
NATIVE_ENV_STR = " ".join(["{}={}".format(e[0], e[1]) for e in _NATIVE_ENV_TUPLES])

EMSCRIPTEN_VERSION = "1.38.28"
EMSCRIPTEN_DIR = "/usr/local/code/lib/emsdk/emscripten/{}/".format(EMSCRIPTEN_VERSION)
EMSCRIPTEN_CMAKE_TOOLCHAIN = join(EMSCRIPTEN_DIR, "cmake", "Modules", "Platform", "Emscripten.cmake")
INITIAL_PATH = os.environ["PATH"]
EMSCRIPTEN_ENV_DICT = {
    "PATH": "{}:{}".format(INITIAL_PATH, EMSCRIPTEN_DIR)
}
EMSCRIPTEN_SYSROOT = join(FAASM_HOME, "emsysroot")

BUILD_DIR = join(PROJ_ROOT, "work")

CONFIG_FLAGS = [
    "--target={}".format(CONFIG_TARGET),
    "--host={}".format(CONFIG_TARGET),
    "--prefix={}".format(SYSROOT),
]

# ---------------------------------------------
# Utility functions
# ---------------------------------------------

def get_wasm_func_path(user, func_name):
    func_dir = join(WASM_DIR, user, func_name)

    if not exists(func_dir):
        makedirs(func_dir, exist_ok=True, mode=0o775)

    return join(func_dir, "function.wasm")


def playbook_command(playbook_name, sudo=True, inventory=None):
    cmd = [
        "ansible-playbook",
        playbook_name,
        "--ask-become-pass" if sudo else "",
        "-i inventory/{}".format(inventory) if inventory else "",
    ]

    cmd = " ".join(cmd)

    print(cmd)
    call(cmd, shell=True, cwd=ANSIBLE_DIR)


def sudo_script(script_name):
    script_path = join("bin", script_name)

    cmd = [
        "sudo",
        "./{}".format(script_path),
    ]
    cmd = " ".join(cmd)

    print(cmd)
    call(cmd, shell=True, cwd=PROJ_ROOT)
