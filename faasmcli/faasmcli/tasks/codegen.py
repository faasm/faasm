from subprocess import run

from invoke import task
from copy import copy
from os import environ, remove
from os.path import join
from sys import exit

from faasmcli.util.codegen import find_codegen_func, find_codegen_shared_lib
from faasmcli.util.env import FAASM_RUNTIME_ROOT

LIB_FAKE_FILES = [
    join(FAASM_RUNTIME_ROOT, "lib", "fake", "libfakeLibA.so"),
    join(FAASM_RUNTIME_ROOT, "lib", "fake", "libfakeLibB.so"),
]

WAMR_WHITELISTED_FUNCS = [
    # Misc
    ["demo", "chain"],
    ["ffmpeg", "check"],
    # Environment
    ["demo", "argc_argv_test"],
    ["demo", "exit"],
    ["demo", "getenv"],
    # Filesystem
    ["demo", "fcntl"],
    ["demo", "file"],
    ["demo", "filedescriptor"],
    ["demo", "fstat"],
    ["demo", "fread"],
    # Input output
    ["demo", "check_input"],
    ["demo", "echo"],
    ["demo", "stdout"],
    ["demo", "stderr"],
]

SGX_WHITELISTED_FUNCS = [
    ["demo", "hello"],
    ["demo", "chain_named_a"],
    ["demo", "chain_named_b"],
    ["demo", "chain_named_c"],
]


@task(default=True)
def codegen(ctx, user, function, wamr=False, sgx=False):
    """
    Generates machine code for the given function
    """
    env = copy(environ)
    env.update(
        {
            "WASM_VM": "wamr" if wamr else "wavm",
            "LD_LIBRARY_PATH": "/usr/local/lib/",
        }
    )

    if (not wamr) and sgx:
        print("Can't run SGX codegen with WAVM. Add --wamr flag.")
        exit(1)

    binary = find_codegen_func()
    run(
        "{} {} {} {}".format(binary, user, function, "--sgx" if sgx else ""),
        shell=True,
        env=env,
        check=True,
    )


@task
def user(ctx, user):
    """
    Generates machine for all the functions belonging to the given user
    """
    _do_codegen_user(user)


def _do_codegen_user(user):
    print("Running WAVM codegen for user {}".format(user))

    binary = find_codegen_func()
    env = copy(environ)
    env.update(
        {
            "WASM_VM": "wavm",
            "LD_LIBRARY_PATH": "/usr/local/lib/",
        }
    )

    run("{} {}".format(binary, user), shell=True, env=env, check=True)


def _do_codegen_file(path):
    binary = find_codegen_shared_lib()

    env = copy(environ)
    env.update(
        {
            "LD_LIBRARY_PATH": "/usr/local/lib/",
        }
    )
    run("{} {}".format(binary, path), env=env, shell=True, check=True)


@task
def local(ctx):
    """
    Runs codegen on functions used in tests
    """
    _do_codegen_user("demo")
    _do_codegen_user("errors")
    _do_codegen_user("ffmpeg")
    _do_codegen_user("mpi")
    _do_codegen_user("omp")
    _do_codegen_user("python")

    # Do codegen for libfake
    for so in LIB_FAKE_FILES:
        _do_codegen_file(so)

    # Run the WAMR codegen required by the tests
    for user, func in WAMR_WHITELISTED_FUNCS:
        codegen(ctx, user, func, wamr=True)

    # Run the SGX codegen required by the tests
    for user, func in SGX_WHITELISTED_FUNCS:
        codegen(ctx, user, func, wamr=True, sgx=True)


# 29/09/2021 - TODO: remove task when WAMR and SGX development is consolidated
# This task is used in the tests, consequently we only need to delete the object
# files from local storage, not minio.
@task
def remove_wamr_codegen(ctx):
    """
    Clean WAMR generated code
    """
    LOCAL_OBJECT_STORE_PREFIX = "/usr/local/faasm/object"
    WAMR_FUNC_NAME = "function.aot"
    SGX_FUNC_NAME = "function.aot.sgx"

    print("Removing local WAMR machine code")
    for user, func in WAMR_WHITELISTED_FUNCS:
        file_path = join(LOCAL_OBJECT_STORE_PREFIX, user, func, WAMR_FUNC_NAME)
        remove(file_path)

    print("Removing local SGX machine code")
    for user, func in SGX_WHITELISTED_FUNCS:
        file_path = join(LOCAL_OBJECT_STORE_PREFIX, user, func, SGX_FUNC_NAME)
        remove(file_path)
