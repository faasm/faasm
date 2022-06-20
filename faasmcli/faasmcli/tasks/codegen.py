from subprocess import run

from invoke import task
from copy import copy
from os import environ
from os.path import join

from faasmcli.util.codegen import find_codegen_func, find_codegen_shared_lib
from faasmcli.util.env import FAASM_RUNTIME_ROOT

LIB_FAKE_FILES = [
    join(FAASM_RUNTIME_ROOT, "lib", "fake", "libfakeLibA.so"),
    join(FAASM_RUNTIME_ROOT, "lib", "fake", "libfakeLibB.so"),
]

WAMR_ALLOWED_FUNCS = [
    # Misc
    ["demo", "chain"],
    ["demo", "chain_named_a"],
    ["demo", "chain_named_b"],
    ["demo", "chain_named_c"],
    ["ffmpeg", "check"],
    # Environment
    ["demo", "argc_argv_test"],
    ["demo", "exit"],
    ["demo", "getenv"],
    ["errors", "ret_one"],
    # Memory
    ["demo", "brk"],
    ["demo", "mmap"],
    ["demo", "mmap_big"],
    # Filesystem
    ["demo", "fcntl"],
    ["demo", "file"],
    ["demo", "filedescriptor"],
    ["demo", "fstat"],
    ["demo", "fread"],
    ["demo", "shared_file"],
    # Input output
    ["demo", "check_input"],
    ["demo", "echo"],
    ["demo", "stdout"],
    ["demo", "stderr"],
]

SGX_ALLOWED_FUNCS = [
    ["demo", "hello"],
    ["demo", "chain_named_a"],
    ["demo", "chain_named_b"],
    ["demo", "chain_named_c"],
    # Environment
    ["demo", "argc_argv_test"],
]


@task(default=True)
def codegen(ctx, user, function):
    """
    Generates machine code for the given function
    """
    env = copy(environ)
    env.update(
        {
            "LD_LIBRARY_PATH": "/usr/local/lib/",
        }
    )

    binary = find_codegen_func()
    run(
        "{} {} {}".format(binary, user, function),
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

    # For WAMR and SGX codegen, we need to update the environment
    env = copy(environ)

    # Run the WAMR codegen required by the tests
    env.update({"WASM_VM": "wamr"})
    for user, func in WAMR_ALLOWED_FUNCS:
        codegen(ctx, user, func)

    # Run the SGX codegen required by the tests
    env.update({"WASM_VM": "sgx"})
    for user, func in SGX_ALLOWED_FUNCS:
        codegen(ctx, user, func)
