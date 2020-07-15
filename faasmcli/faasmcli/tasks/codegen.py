from multiprocessing.pool import Pool
from os.path import join
from os import listdir
from subprocess import check_output

from invoke import task

from faasmcli.util.codegen import find_codegen_func, find_codegen_shared_lib
from faasmcli.util.env import FAASM_RUNTIME_ROOT, WASM_DIR, FAASM_MACHINE_CODE_DIR, THIRD_PARTY_DIR
from faasmcli.util.shell import find_command


# This is hacked together as a PoC as WAMR codegen is not fully integrated
def _do_wamr_codegen(user, function):
    print("Running WAMR codegen for {}/{}".format(user, function))

    binary = find_command("wamrc", dirs=[THIRD_PARTY_DIR, "wamr", "wamr-compiler", "build"])
    func_file = join(WASM_DIR, user, function, "function.wasm")
    output_file = join(FAASM_MACHINE_CODE_DIR, user, function, "function.aot")

    cmd = "{} -o {} {}".format(binary, output_file, func_file)
    check_output(cmd, shell=True)


@task(default=True)
def codegen(ctx, user, function, wamr=False):
    """
    Generates machine code for the given function
    """
    if wamr:
        _do_wamr_codegen(user, function)
    else:
        binary = find_codegen_func()
        check_output("{} {} {}".format(binary, user, function), shell=True)


@task
def user(ctx, user, wamr=False):
    """
    Generates machine for all the functions belonging to the given user
    """
    if wamr:
        user_dir = join(WASM_DIR, user)
        for func_name in listdir(user_dir):
            _do_wamr_codegen(user, func_name)
    else:
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
    _do_codegen_user("rust")

    # Run these in parallel
    p = Pool(3)
    users = ["python", "sgd", "tf"]
    p.map(_do_codegen_user, users)

    print("Running codegen on python shared objects")
    binary = find_codegen_shared_lib()
    shared_obj_dir = join(FAASM_RUNTIME_ROOT, "lib", "python3.7")
    check_output("{} {}".format(binary, shared_obj_dir), shell=True)
