from subprocess import run

from invoke import task
from copy import copy
from os import environ
from os.path import join

from faasmcli.util.codegen import find_codegen_func, find_codegen_shared_lib
from faasmcli.util.env import (
    FAASM_RUNTIME_ROOT,
    PY_RUNTIME_ROOT,
    PYTHON_FUNC,
    PYTHON_USER,
)

LIB_FAKE_FILES = [
    join(FAASM_RUNTIME_ROOT, "lib", "fake", "libfakeLibA.so"),
    join(FAASM_RUNTIME_ROOT, "lib", "fake", "libfakeLibB.so"),
]


@task(default=True)
def codegen(ctx, user, function, clean=False):
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
    codegen_cmd = [
        binary,
        user,
        "--func {}".format(function),
        "--clean" if clean else "",
    ]
    codegen_cmd = " ".join(codegen_cmd)
    run(codegen_cmd, shell=True, env=env, check=True)


@task
def user(ctx, user, clean=False):
    """
    Generates machine for all the functions belonging to the given user
    """
    _do_codegen_user(user, clean)


def _do_codegen_user(user, clean=False):
    print("Running codegen for user {}".format(user))

    binary = find_codegen_func()
    env = copy(environ)
    env.update(
        {
            "LD_LIBRARY_PATH": "/usr/local/lib/",
        }
    )

    codegen_cmd = [
        binary,
        user,
        "--clean" if clean else "",
    ]
    codegen_cmd = " ".join(codegen_cmd)
    run(codegen_cmd, shell=True, env=env, check=True)


def _do_codegen_shared_lib(path, clean=False):
    binary = find_codegen_shared_lib()

    env = copy(environ)
    env.update(
        {
            "LD_LIBRARY_PATH": "/usr/local/lib/",
        }
    )
    codegen_cmd = [
        binary,
        path,
        "--clean" if clean else "",
    ]
    codegen_cmd = " ".join(codegen_cmd)
    run(codegen_cmd, env=env, shell=True, check=True)


@task
def libs(ctx, clean=False):
    """
    Run codegen for shared libraries
    """
    env = copy(environ)
    env.update({"WASM_VM": "wavm"})
    for so in LIB_FAKE_FILES:
        _do_codegen_shared_lib(so, clean)


@task
def wavm(ctx, clean=False):
    """
    Run codegen for shared libraries
    """
    env = copy(environ)
    env.update({"WASM_VM": "wavm"})
    _do_codegen_user("demo", clean)
    _do_codegen_user("errors", clean)
    _do_codegen_user("mpi", clean)
    _do_codegen_user("omp", clean)


@task
def wamr(ctx, clean=False):
    """
    Run WAMR codegen
    """
    env = copy(environ)
    env.update({"WASM_VM": "wamr"})
    _do_codegen_user("demo", clean)


@task
def sgx(ctx, clean=False):
    """
    Run SGX codegen
    """
    env = copy(environ)
    env.update({"WASM_VM": "sgx"})
    _do_codegen_user("demo", clean)


@task
def python(ctx, clean=False):
    """
    Run Python codegen
    """
    env = copy(environ)
    env.update({"WASM_VM": "wavm"})
    codegen(ctx, PYTHON_USER, PYTHON_FUNC, clean)
    _do_codegen_shared_lib(PY_RUNTIME_ROOT, clean)


@task
def tests(ctx, clean=False):
    """
    Runs codegen for all WASM needed for the tests
    """
    # Run the WAVM codegen
    wavm(ctx, clean)

    # Do codegen for shared libraries
    libs(ctx, clean)

    # Run the WAMR codegen required by the tests
    wamr(ctx, clean)

    # Run the SGX codegen required by the tests
    sgx(ctx, clean)

    # Run the Python codegen
    python(ctx, clean)
