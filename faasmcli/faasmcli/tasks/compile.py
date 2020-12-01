from os import listdir
from os.path import join, splitext

from invoke import task

from faasmcli.util.env import FUNC_DIR, PROJ_ROOT
from faasmcli.util.compile import wasm_cmake, wasm_copy_upload

FUNC_BUILD_DIR = join(PROJ_ROOT, "build", "func")


def _copy_built_function(user, func):
    src_file = join(FUNC_BUILD_DIR, user, ".".join([func, "wasm"]))
    wasm_copy_upload(user, func, src_file)


@task(default=True, name="compile")
def compile(ctx, user, func, clean=False, debug=False):
    """
    Compile a function
    """
    # Build the function (gets written to the build dir)
    # Will fail if compilation fails
    target = func
    wasm_cmake(FUNC_DIR, FUNC_BUILD_DIR, target, clean, debug)

    _copy_built_function(user, func)


@task
def user(ctx, user, clean=False, debug=False):
    """
    Compile all functions belonging to the given user
    """

    # Build all funcs for this user (will fail if any builds fail)
    target = "{}_all_funcs".format(user)
    wasm_cmake(FUNC_DIR, FUNC_BUILD_DIR, target, clean, debug)

    # Work out all the functions for this user (that we assume will have been
    # built)
    for func_file in listdir(join(FUNC_BUILD_DIR, user)):
        name, ext = splitext(func_file)
        if ext != ".wasm":
            continue

        _copy_built_function(user, name)
