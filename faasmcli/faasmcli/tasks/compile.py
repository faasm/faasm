from os import makedirs, listdir
from os.path import join, splitext
from shutil import copy
from subprocess import call

from invoke import task

from faasmcli.util.env import FUNC_DIR, PROJ_ROOT
from faasmcli.util.typescript import ASC_BINARY, TS_DIR
from faasmcli.util.compile import wasm_cmake, wasm_copy_upload

FUNC_BUILD_DIR = join(PROJ_ROOT, "build", "func")


def _copy_built_function(user, func):
    src_file = join(FUNC_BUILD_DIR, user, ".".join([func, "wasm"]))
    wasm_copy_upload(user, func, src_file)


@task(default=True, name="compile")
def compile(ctx, user, func, clean=False, debug=False, ts=False, sgx=False):
    """
    Compile a function
    """

    # Typescript compilation
    if ts:
        return _ts_compile(func)

    # Build the function (gets written to the build dir)
    # Will fail if compilation fails
    target = func
    wasm_cmake(FUNC_DIR, FUNC_BUILD_DIR, target, clean, debug, sgx=sgx)

    _copy_built_function(user, func)


@task
def user(ctx, user, clean=False, debug=False):
    """
    Compile all functions belonging to the given user
    """

    # Build all funcs for this user (will fail if any builds fail)
    target = "{}_all_funcs".format(user)
    wasm_cmake(FUNC_DIR, FUNC_BUILD_DIR, target, clean, debug)

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
