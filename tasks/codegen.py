from multiprocessing.pool import Pool
from os.path import join
from subprocess import check_output

from invoke import task

from tasks.util.codegen import find_codegen_func, find_codegen_shared_lib
from tasks.util.env import FAASM_RUNTIME_ROOT, FAASM_LOCAL_DIR


@task(default=True)
def codegen(ctx, user, function):
    """
    Generates machine code for the given function
    """
    binary = find_codegen_func()
    check_output("{} {} {}".format(binary, user, function), shell=True)


@task
def user(ctx, user):
    """
    Generates machine for all the functions belonging to the given user
    """
    _do_codegen_user(user)


def _do_codegen_user(user):
    print("Running codegen for user {}".format(user))

    binary = find_codegen_func()
    check_output("{} {}".format(binary, user), shell=True)


@task
def local(ctx):
    """
    Runs codegen on functions used in tests
    """
    _do_codegen_user("demo")
    _do_codegen_user("errors")
    _do_codegen_user("omp")
    _do_codegen_user("mpi")

    # Run these in parallel
    p = Pool(3)
    users = ["python", "sgd", "tf"]
    p.map(_do_codegen_user, users)

    print("Running codegen on python shared objects")
    binary = find_codegen_shared_lib()
    shared_obj_dir = join(FAASM_RUNTIME_ROOT, "lib", "python3.7")
    check_output("{} {}".format(binary, shared_obj_dir), shell=True)
