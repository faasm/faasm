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


@task(default=True)
def codegen(ctx, user, function, wamr=False):
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

    binary = find_codegen_func()
    run(
        "{} {} {}".format(binary, user, function),
        shell=True,
        env=env,
        check=True,
    )


@task
def user(ctx, user, wamr=False):
    """
    Generates machine for all the functions belonging to the given user
    """
    _do_codegen_user(user, wamr=wamr)


def _do_codegen_user(user, wamr=False):
    print("Running codegen for user {}".format(user))

    binary = find_codegen_func()
    env = copy(environ)
    env.update(
        {
            "WASM_VM": "wamr" if wamr else "wavm",
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
def local(ctx, wamr=False):
    """
    Runs codegen on functions used in tests
    """
    _do_codegen_user("demo", wamr=wamr)
    _do_codegen_user("errors", wamr=wamr)
    _do_codegen_user("mpi", wamr=wamr)
    _do_codegen_user("omp", wamr=wamr)
    _do_codegen_user("python", wamr=wamr)

    # Do codegen for libfake
    for so in LIB_FAKE_FILES:
        _do_codegen_file(so)

    # Run the WAMR codegen required by the tests
    codegen(ctx, "demo", "echo", wamr=True)
    codegen(ctx, "demo", "chain", wamr=True)
