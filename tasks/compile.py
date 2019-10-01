from subprocess import call

from invoke import task

from tasks.util.env import FAASM_TOOLCHAIN_FILE, FUNC_BUILD_DIR, FUNC_DIR
from tasks.util.files import clean_dir
from tasks.util.typescript import ASC_BINARY, TS_DIR


@task
def compile(context, clean=False, func=None, debug=False, user=None, ts=False):
    # Typescript compilation
    if ts:
        return _ts_compile(func)

    build_type = "wasm"
    cmake_build_type = "Debug" if debug else "Release"

    clean_dir(FUNC_BUILD_DIR, clean)

    build_cmd = [
        "cmake",
        "-DFAASM_BUILD_TYPE={}".format(build_type),
        "-DCMAKE_TOOLCHAIN_FILE={}".format(FAASM_TOOLCHAIN_FILE),
        "-DCMAKE_BUILD_TYPE={}".format(cmake_build_type),
        FUNC_DIR,
    ]

    res = call(" ".join(build_cmd), shell=True, cwd=FUNC_BUILD_DIR)
    if res != 0:
        print("Failed to compile")
        return

    target = ""
    if func:
        target = func
    elif user:
        target = "{}_all_funcs".format(user)

    cmd = "VERBOSE=1 make {}".format(target) if target else "make -j"
    call(cmd, shell=True, cwd=FUNC_BUILD_DIR)


def _ts_compile(func, optimize=True):
    cmd = [
        ASC_BINARY,
        "assembly/{}.ts".format(func),
        "-b build/{}.wasm".format(func),
        "-t build/{}.wast".format(func),
        "--sourceMap",
        "--validate",
    ]

    if optimize:
        cmd.append("--optimize")
    else:
        cmd.append("--debug")

    cmd_string = " ".join(cmd)
    print(cmd_string)
    call(cmd_string, cwd=TS_DIR, shell=True)
