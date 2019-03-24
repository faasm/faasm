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
PYTHON_FUNC_BUILD_DIR = join(PROJ_ROOT, "func", "python_func_build")

WASM_DIR = join(PROJ_ROOT, "wasm")

RUNTIME_S3_BUCKET = "faasm-runtime"
STATE_S3_BUCKET = "faasm-state"

# TODO - avoid hard-coding
AWS_ACCOUNT_ID = "733781933474"
AWS_REGION = "eu-west-1"

# ---------------------------------------------
# Compile related
# ---------------------------------------------

WASM_LIB_DIR = join(PROJ_ROOT, "wasm", "lib")

SYSROOT = join(FAASM_HOME, "sysroot")

TARGET_TRIPLE = "wasm32-unknown-unknown-wasm"
CONFIG_TARGET = "wasm32"
CONFIG_HOST = "wasm32-unknown-none"

COMPILER_FLAGS = [
    "--target={}".format(TARGET_TRIPLE),
    "--sysroot={}".format(SYSROOT),
]

COMPILER_FLAGS_STRING = " ".join(COMPILER_FLAGS)
COMPILER_FLAGS_STRING = "\"{}\"".format(COMPILER_FLAGS_STRING)

PYODIDE_ROOT = "/usr/local/code/pyodide/"
PY_EMSCRIPTEN_DIR = join(PYODIDE_ROOT, "emsdk/emsdk/emscripten/tag-1.38.22")
PY_EMSCRIPTEN_CMAKE_TOOLCHAIN = join(PY_EMSCRIPTEN_DIR, "cmake", "Modules", "Platform", "Emscripten.cmake")

WASM_TOOLCHAIN = join(PROJ_ROOT, "WasmToolchain.cmake")

INITIAL_PATH = os.environ["PATH"]

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
