from os import makedirs, environ
from os.path import dirname, realpath, join, exists, expanduser

from faasmtools.build import FAASM_LOCAL_DIR

HOME_DIR = expanduser("~")
PROJ_ROOT = dirname(dirname(dirname(dirname(realpath(__file__)))))
THIRD_PARTY_DIR = join(PROJ_ROOT, "third-party")
ANSIBLE_ROOT = join(PROJ_ROOT, "ansible")

PYTHON_USER = "python"
PYTHON_FUNC = "py_func"

FAASM_HOME = join(HOME_DIR, "faasm")
FAASM_DATA_DIR = join(HOME_DIR, "faasm", "data")
FAASM_MACHINE_CODE_DIR = join(FAASM_LOCAL_DIR, "object")
FAASM_RUNTIME_ROOT = join(FAASM_LOCAL_DIR, "runtime_root")
FAASM_SHARED_ROOT = join(FAASM_LOCAL_DIR, "shared")
FAASM_SHARED_STORAGE_ROOT = join(FAASM_LOCAL_DIR, "shared_store")
FAASM_SHARED_ROOT = join(FAASM_LOCAL_DIR, "shared")
FAASM_INSTALL_DIR = "/usr/local"

FAASM_CONFIG_FILE = join(FAASM_HOME, "faasm.ini")

BENCHMARK_BUILD = join(HOME_DIR, "faasm", "bench")
RESULT_DIR = join(FAASM_HOME, "results")
BENCHMARK_ENV = {
    "CGROUP_MODE": "off",
    "NETNS_MODE": "off",
    "GLOBAL_MESSAGE_TIMEOUT": "120000",
    "LOG_LEVEL": "off",
}

FUNC_DIR = join(PROJ_ROOT, "func")

WASM_DIR = join(PROJ_ROOT, "wasm")
WASM_LIB_DIR = join(PROJ_ROOT, "wasm", "lib")

RUNTIME_S3_BUCKET = "faasm-runtime"
MISC_S3_BUCKET = "faasm-misc"
DATA_S3_BUCKET = "faasm-data"
TEST_S3_BUCKET = "faasm-test"

# TODO - avoid hard-coding
AWS_ACCOUNT_ID = "733781933474"
AWS_REGION = "eu-west-1"

PY_RUNTIME_ROOT = join(FAASM_RUNTIME_ROOT, "lib", "python3.8")

LATEST_CMAKE = "/usr/local/lib/cmake-3.15/bin/cmake"

POSSIBLE_BUILD_BINS = [
    "/faasm/build/bin/",  # Containers
    "/faasm/build/third-party/WAVM/bin/",  # Containers
    join(PROJ_ROOT, "build", "bin"),  # Local builds
    join(PROJ_ROOT, "build", "cmake", "bin"),  # Local builds
    join(PROJ_ROOT, "build", "third-party", "WAVM", "bin"),  # Local builds
    join(
        PROJ_ROOT, "build", "cmake", "third-party", "WAVM", "bin"
    ),  # Local builds
    join(PROJ_ROOT, "cmake-build-debug", "bin"),  # CLion
    join(
        PROJ_ROOT, "cmake-build-debug", "third-party", "WAVM", "bin"
    ),  # CLion
    join(HOME_DIR, "faasm", "bench", "bin"),  # Benchmark
]


def get_wasm_func_path(user, func_name):
    func_dir = join(WASM_DIR, user, func_name)

    if not exists(func_dir):
        makedirs(func_dir, exist_ok=True, mode=0o775)

    return join(func_dir, "function.wasm")


def set_benchmark_env():
    environ.update(BENCHMARK_ENV)
