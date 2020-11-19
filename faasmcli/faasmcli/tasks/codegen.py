from subprocess import run

from invoke import task
from copy import copy
from os import environ

from faasmcli.util.codegen import find_codegen_func


@task(default=True)
def codegen(ctx, user, function, wamr=False):
    """
    Generates machine code for the given function
    """
    env = copy(environ)
    env.update({"WASM_VM": "wamr" if wamr else "wavm"})

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
    env = {"WASM_VM": "wamr" if wamr else "wavm"}

    run("{} {}".format(binary, user), shell=True, env=env, check=True)


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

    # Always run the codegen required by the tests
    codegen(ctx, "demo", "echo", wamr=True)
    codegen(ctx, "demo", "chain", wamr=True)
