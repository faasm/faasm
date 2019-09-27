from os import makedirs, environ
from os.path import dirname, realpath, join, exists, expanduser

HOME_DIR = expanduser("~")
PROJ_ROOT = dirname(dirname(dirname(realpath(__file__))))
THIRD_PARTY_DIR = join(PROJ_ROOT, "third-party")
ANSIBLE_ROOT = join(PROJ_ROOT, "ansible")

FAASM_HOME = join(HOME_DIR, "faasm")
FAASM_LOCAL_DIR = "/usr/local/faasm"
FAASM_RUNTIME_ROOT = join(FAASM_LOCAL_DIR, "runtime_root")
FAASM_INSTALL_DIR = join(FAASM_LOCAL_DIR, "install")

BENCHMARK_BUILD = join(HOME_DIR, "faasm", "bench")
RESULT_DIR = join(FAASM_HOME, "results")
BENCHMARK_ENV = {
    "CGROUP_MODE": "off",
    "NETNS_MODE": "off",
    "FS_MODE": "on",
    "GLOBAL_MESSAGE_TIMEOUT": "120000",
    "LOG_LEVEL": "off",
}

FUNC_DIR = join(PROJ_ROOT, "func")
FUNC_BUILD_DIR = join(PROJ_ROOT, "build", "func")

WASM_DIR = join(PROJ_ROOT, "wasm")
WASM_LIB_DIR = join(PROJ_ROOT, "wasm", "lib")

RUNTIME_S3_BUCKET = "faasm-runtime"
STATE_S3_BUCKET = "faasm-state"
MISC_S3_BUCKET = "faasm-misc"
DATA_S3_BUCKET = "faasm-data"

# TODO - avoid hard-coding
AWS_ACCOUNT_ID = "733781933474"
AWS_REGION = "eu-west-1"

PYODIDE_ROOT = join(PROJ_ROOT, "third-party", "pyodide")
PYODIDE_PACKAGES = join(PYODIDE_ROOT, "packages")
PYODIDE_INSTALL_DIR = join(PYODIDE_ROOT, "cpython", "installs", "python-3.7.0")
PY_RUNTIME_ROOT = join(FAASM_RUNTIME_ROOT, "lib", "python3.7")

TOOLCHAIN_ROOT = join(PROJ_ROOT, "toolchain")
FAASM_TOOLCHAIN_FILE = join(TOOLCHAIN_ROOT, "FaasmToolchain.cmake")
FAASM_SYSROOT = join(FAASM_LOCAL_DIR, "llvm-sysroot")

LATEST_CMAKE = "/usr/local/lib/cmake-3.15/bin/cmake"


def get_wasm_func_path(user, func_name):
    func_dir = join(WASM_DIR, user, func_name)

    if not exists(func_dir):
        makedirs(func_dir, exist_ok=True, mode=0o775)

    return join(func_dir, "function.wasm")


def set_benchmark_env():
    environ.update(BENCHMARK_ENV)
