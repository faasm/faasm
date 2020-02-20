from multiprocessing.pool import Pool
from os import makedirs
from os import remove
from os.path import exists, join
from shutil import rmtree
from subprocess import check_output

from invoke import task

from tasks.python import run_python_codegen
from tasks.util.codegen import find_codegen_func, find_codegen_shared_lib
from tasks.util.env import FAASM_RUNTIME_ROOT, FAASM_LOCAL_DIR
from tasks.util.env import FAASM_SYSROOT
from tasks.util.release import get_runtime_tar_name, get_runtime_tar_path, \
    get_sysroot_tar_name, get_toolchain_tar_name, get_toolchain_tar_path, get_sysroot_tar_path, get_toolchain_url, \
    get_runtime_url, get_sysroot_url
from tasks.util.upload_util import download_tar_from_url

TOOLCHAIN_INSTALL = join(FAASM_LOCAL_DIR, "toolchain")


def _do_codegen_for_user(user):
    print("Running codegen for user {}".format(user))

    binary = find_codegen_func()
    check_output("{} {}".format(binary, user), shell=True)


@task
def run_local_codegen(ctx):
    _do_codegen_for_user("demo")
    _do_codegen_for_user("errors")
    _do_codegen_for_user("omp")
    _do_codegen_for_user("mpi")

    # Run these in parallel
    p = Pool(3)
    users = ["python", "sgd", "tf"]
    p.map(_do_codegen_for_user, users)

    print("Running codegen on python shared objects")
    binary = find_codegen_shared_lib()
    shared_obj_dir = join(FAASM_RUNTIME_ROOT, "lib", "python3.7")
    check_output("{} {}".format(binary, shared_obj_dir), shell=True)


@task
def download_sysroot(ctx):
    url = get_sysroot_url()
    tar_name = get_sysroot_tar_name()
    tar_path = get_sysroot_tar_path()

    if not exists(FAASM_LOCAL_DIR):
        makedirs(FAASM_LOCAL_DIR)

    if exists(FAASM_SYSROOT):
        print("Deleting existing sysroot at {}".format(FAASM_SYSROOT))
        check_output("rm -rf {}".format(FAASM_SYSROOT), shell=True)

    print("Downloading sysroot archive")
    download_tar_from_url(url, tar_name, FAASM_LOCAL_DIR)

    print("Removing downloaded archive")
    remove(tar_path)


@task
def download_toolchain(ctx):
    url = get_toolchain_url()
    tar_name = get_toolchain_tar_name()
    tar_path = get_toolchain_tar_path()

    if exists(TOOLCHAIN_INSTALL):
        print("Deleting existing toolchain at {}".format(TOOLCHAIN_INSTALL))
        check_output("rm -rf {}".format(TOOLCHAIN_INSTALL), shell=True)

    if not exists(FAASM_LOCAL_DIR):
        makedirs(FAASM_LOCAL_DIR)

    print("Downloading toolchain archive")
    download_tar_from_url(url, tar_name, FAASM_LOCAL_DIR)

    print("Removing downloaded archive")
    remove(tar_path)


@task
def download_runtime_root(ctx, nocodegen=False):
    url = get_runtime_url()
    tar_name = get_runtime_tar_name()
    tar_path = get_runtime_tar_path()

    # Clear out existing
    if exists(FAASM_RUNTIME_ROOT):
        print("Removing existing")
        rmtree(FAASM_RUNTIME_ROOT)

    # Download the bundle
    print("Downloading from S3")
    download_tar_from_url(url, tar_name, FAASM_LOCAL_DIR)

    # Remove downloaded tar
    remove(tar_path)

    # Run codegen
    if not nocodegen:
        print("Running codegen")
        run_python_codegen(ctx)
