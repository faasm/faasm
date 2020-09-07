from multiprocessing.pool import Pool
from os.path import join
from subprocess import check_output, run

from invoke import task

from faasmcli.util.codegen import find_codegen_func, find_codegen_shared_lib
from faasmcli.util.env import FAASM_RUNTIME_ROOT


@task(default=True)
def codegen(ctx, user, function, wamr=False):
    """
    Generates machine code for the given function
    """
    env = {"WASM_VM": "wamr" if wamr else "wavm"}

    binary = find_codegen_func()
    run("{} {} {}".format(binary, user, function), shell=True, env=env, check=True)


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
    _do_codegen_user("omp", wamr=wamr)
    _do_codegen_user("mpi", wamr=wamr)
    _do_codegen_user("rust", wamr=wamr)

    # Run these in parallel
    p = Pool(3)
    users = [
        ("python", wamr),
        ("sgd", wamr),
        ("tf", wamr),
    ]
    p.starmap(_do_codegen_user, users)

    print("Running codegen on python shared objects")
    binary = find_codegen_shared_lib()
    shared_obj_dir = join(FAASM_RUNTIME_ROOT, "lib", "python3.7")
    check_output("{} {}".format(binary, shared_obj_dir), shell=True)
