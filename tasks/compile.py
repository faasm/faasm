from subprocess import call

from invoke import task

from tasks.util.env import FAASM_TOOLCHAIN_FILE, FUNC_BUILD_DIR
from tasks.util.files import clean_dir


@task
def compile(context, clean=False, func=None, debug=False, user=None):
    build_type = "wasm"
    cmake_build_type = "Debug" if debug else "Release"

    clean_dir(FUNC_BUILD_DIR, clean)

    build_cmd = [
        "cmake",
        "-DFAASM_BUILD_TYPE={}".format(build_type),
        "-DCMAKE_TOOLCHAIN_FILE={}".format(FAASM_TOOLCHAIN_FILE),
        "-DCMAKE_BUILD_TYPE={}".format(cmake_build_type),
        ".."
    ]

    res = call(" ".join(build_cmd), shell=True, cwd=FUNC_BUILD_DIR)
    if res != 0:
        print("Failed to compile")
        return

    # Allow specifying a single function
    target = None
    if func:
        target = func
    elif user:
        target = "{}_all_funcs".format(user)

    cmd = "make {}".format(target) if target else "make -j"
    call(cmd, shell=True, cwd=FUNC_BUILD_DIR)
