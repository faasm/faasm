from os import makedirs, environ
from os.path import dirname, realpath, join, exists, expanduser

from faasmtools.build import FAASM_LOCAL_DIR


def _get_dir(variable, default):
    env_val = environ.get(variable)
    return env_val if env_val else default


HOME_DIR = expanduser("~")
PROJ_ROOT = dirname(dirname(dirname(realpath(__file__))))
DOCS_ROOT = join(PROJ_ROOT, "docs")

PYTHON_USER = "python"
PYTHON_FUNC = "py_func"

FAASM_MACHINE_CODE_DIR = join(FAASM_LOCAL_DIR, "object")
FAASM_RUNTIME_ROOT = join(FAASM_LOCAL_DIR, "runtime_root")
FAASM_SHARED_ROOT = join(FAASM_LOCAL_DIR, "shared")
WASM_DIR = join(FAASM_LOCAL_DIR, "wasm")

FAASM_BUILD_DIR = _get_dir("FAASM_BUILD_DIR", "/build/faasm")
FAASM_CONAN_CACHE = "/build/conan-cache"
FAASM_CONAN_PROFILES = join(PROJ_ROOT, "conan-profiles")
FAASM_INSTALL_DIR = _get_dir("FAASM_INSTALL_DIR", "/usr/local")

FAASM_CONFIG_FILE = join(PROJ_ROOT, "faasm.ini")
GLOBAL_FAASM_CONFIG_DIR = join(HOME_DIR, ".config")
GLOBAL_FAASM_CONFIG_FILE = join(GLOBAL_FAASM_CONFIG_DIR, "faasm.ini")

PY_RUNTIME_ROOT = join(FAASM_RUNTIME_ROOT, "lib", "python3.8")

FAASM_SGX_MODE_DISABLED = "Disabled"
FAASM_SGX_MODE_SIM = "Simulation"
FAASM_SGX_MODE_HARDWARE = "Hardware"

# TODO: this variable is duplicated in faabric
LLVM_MAJOR_VERSION = 17


def get_wasm_func_path(user, func_name):
    func_dir = join(WASM_DIR, user, func_name)

    if not exists(func_dir):
        makedirs(func_dir, exist_ok=True, mode=0o775)

    return join(func_dir, "function.wasm")
