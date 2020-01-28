from subprocess import call
from os.path import join

from invoke import task

from tasks.util.env import FAASM_TOOLCHAIN_FILE, FUNC_BUILD_DIR, FUNC_DIR, WASM_DIR
from tasks.util.files import clean_dir
from tasks.util.typescript import ASC_BINARY, TS_DIR



def _do_compile(target, clean, debug):
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

    cmd = "make {}".format(target) if target else "make -j"
    cmd = "VERBOSE=1 {}".format(cmd) if debug else cmd
    res = call(cmd, shell=True, cwd=FUNC_BUILD_DIR)

    if res != 0:
        print("Failed to make")
        return


@task
def compile(ctx, user, func, clean=False, debug=False, ts=False, cp=False):
    # Typescript compilation
    if ts:
        return _ts_compile(func)

    target = func
    _do_compile(target, clean, debug)
    if cp:
        cmd = [
            "cp",
            join(FUNC_BUILD_DIR, user, ".".join([func, "wasm"])),
            join(WASM_DIR, user, func, "function.wasm"),
        ]
        call(" ".join(cmd), shell=True, cwd=FUNC_BUILD_DIR)

@task
def compile_user(ctx, user, clean=False, debug=False):
    target = "{}_all_funcs".format(user)
    _do_compile(target, clean, debug)

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
