from os import mkdir, listdir
from os.path import exists, join, splitext
from shutil import copy
from subprocess import call

from invoke import task

from tasks.util.env import FAASM_TOOLCHAIN_FILE, FUNC_DIR, WASM_DIR, PROJ_ROOT
from tasks.util.files import clean_dir
from tasks.util.typescript import ASC_BINARY, TS_DIR

FUNC_BUILD_DIR = join(PROJ_ROOT, "build", "func")


def _copy_built_function(user, func):
    dest_folder = join(WASM_DIR, user, func)

    if not exists(dest_folder):
        mkdir(dest_folder)

    src_file = join(FUNC_BUILD_DIR, user, ".".join([func, "wasm"]))
    dest_file = join(dest_folder, "function.wasm")

    copy(src_file, dest_file)


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
        raise RuntimeError("Failed on cmake for {}".format(target))

    cmd = "make {}".format(target) if target else "make -j"
    cmd = "VERBOSE=1 {}".format(cmd) if debug else cmd
    res = call(cmd, shell=True, cwd=FUNC_BUILD_DIR)

    if res != 0:
        raise RuntimeError("Failed on make for {}".format(target))


@task
def compile(ctx, user, func, clean=False, debug=False, ts=False):
    # Typescript compilation
    if ts:
        return _ts_compile(func)

    # Build the function (gets written to the build dir)
    # Will fail if compilation fails
    target = func
    _do_compile(target, clean, debug)

    _copy_built_function(user, func)


@task
def compile_user(ctx, user, clean=False, debug=False):
    # Build all funcs for this user (will fail if any builds fail)
    target = "{}_all_funcs".format(user)
    _do_compile(target, clean, debug)

    # Work out all the functions for this user (that we assume will have been built)
    for func_file in listdir(join(FUNC_BUILD_DIR, user)):
        name, ext = splitext(func_file)
        if ext != ".wasm":
            continue

        _copy_built_function(user, name)

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
