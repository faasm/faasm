from os import makedirs
from os.path import dirname, realpath, join, exists, expanduser
from subprocess import call

HOME_DIR = expanduser("~")
PROJ_ROOT = dirname(dirname(realpath(__file__)))

FAASM_HOME = join(HOME_DIR, "faasm")
FAASM_LOCAL_DIR = "/usr/local/faasm"
FAASM_RUNTIME_ROOT = join(FAASM_LOCAL_DIR, "runtime_root")

ANSIBLE_DIR = join(PROJ_ROOT, "ansible")

FUNC_BUILD_DIR = join(PROJ_ROOT, "func", "build")

WASM_DIR = join(PROJ_ROOT, "wasm")
WASM_LIB_DIR = join(PROJ_ROOT, "wasm", "lib")

RUNTIME_S3_BUCKET = "faasm-runtime"
STATE_S3_BUCKET = "faasm-state"
MISC_S3_BUCKET = "faasm-misc"

# TODO - avoid hard-coding
AWS_ACCOUNT_ID = "733781933474"
AWS_REGION = "eu-west-1"

PYODIDE_ROOT = join(PROJ_ROOT, "pyodide")
EMSCRIPTEN_DIR = "/usr/local/faasm/emsdk"
EMSCRIPTEN_TOOLCHAIN = join(EMSCRIPTEN_DIR, "upstream", "latest", "Wack.cmake")
PYODIDE_PACKAGES = join(PYODIDE_ROOT, "packages")
PYODIDE_INSTALL_DIR = join(PYODIDE_ROOT, "cpython", "installs", "python-3.7.0")
PY_RUNTIME_ROOT = join(FAASM_RUNTIME_ROOT, "lib", "python3.7")

WASM_SYSROOT = join(EMSCRIPTEN_DIR, "upstream", "latest", "sysroot")


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
