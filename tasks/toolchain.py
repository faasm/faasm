from multiprocessing.pool import Pool
from os import remove, makedirs
from os.path import join, exists
from subprocess import check_output

from invoke import task

from tasks.util.codegen import find_codegen_func, find_codegen_shared_lib
from tasks.util.env import MISC_S3_BUCKET, FAASM_LOCAL_DIR, FAASM_SYSROOT, FAASM_RUNTIME_ROOT
from tasks.util.upload_util import upload_file_to_s3, download_tar_from_s3
from tasks.util.version import get_faasm_version

TOOLCHAIN_INSTALL = join(FAASM_LOCAL_DIR, "toolchain")


def _get_sysroot_tar_name():
    ver = get_faasm_version()
    return "faasm-sysroot-{}.tar.gz".format(ver)


def _get_sysroot_tar_path():
    tar_name = _get_sysroot_tar_name()
    return join(FAASM_LOCAL_DIR, tar_name)


def _get_toolchain_tar_name():
    ver = get_faasm_version()
    return "faasm-toolchain-{}.tar.gz".format(ver)


def _get_toolchain_tar_path():
    tar_name = _get_toolchain_tar_name()
    return join(FAASM_LOCAL_DIR, tar_name)


def _do_codegen_for_user(user):
    print("Running codegen for user {}".format(user))

    binary = find_codegen_func()
    check_output("{} {}".format(binary, user), shell=True)


@task
def run_local_codegen(ctx):
    _do_codegen_for_user("demo")
    _do_codegen_for_user("errors")

    # Run these in parallel
    p = Pool(3)
    users = ["python", "sgd", "tf"]
    p.map(_do_codegen_for_user, users)

    print("Running codegen on python shared objects")
    binary = find_codegen_shared_lib()
    shared_obj_dir = join(FAASM_RUNTIME_ROOT, "lib", "python3.7")
    check_output("{} {}".format(binary, shared_obj_dir), shell=True)


@task
def backup_toolchain(ctx):
    tar_name = _get_toolchain_tar_name()
    tar_path = _get_toolchain_tar_path()

    print("Creating archive of Faasm toolchain")
    check_output("tar -cf {} toolchain".format(tar_name), shell=True, cwd=FAASM_LOCAL_DIR)

    # Upload
    print("Uploading archive to S3")
    upload_file_to_s3(tar_path, MISC_S3_BUCKET, tar_name, public=True)

    # Remove old tar
    print("Removing archive")
    remove(tar_path)


@task
def backup_sysroot(ctx):
    tar_name = _get_sysroot_tar_name()
    tar_path = _get_sysroot_tar_path()

    print("Creating archive of Faasm sysroot")
    check_output("tar -cf {} llvm-sysroot".format(tar_name), shell=True, cwd=FAASM_LOCAL_DIR)

    # Upload
    print("Uploading archive to S3")
    upload_file_to_s3(tar_path, MISC_S3_BUCKET, tar_name, public=True)

    # Remove old tar
    print("Removing archive")
    remove(tar_path)


@task
def download_sysroot(ctx):
    tar_name = _get_sysroot_tar_name()
    tar_path = _get_sysroot_tar_path()

    if not exists(FAASM_LOCAL_DIR):
        makedirs(FAASM_LOCAL_DIR)

    if exists(FAASM_SYSROOT):
        print("Deleting existing sysroot at {}".format(FAASM_SYSROOT))
        check_output("rm -rf {}".format(FAASM_SYSROOT), shell=True)

    print("Downloading sysroot archive")
    download_tar_from_s3(MISC_S3_BUCKET, tar_name, FAASM_LOCAL_DIR, boto=False)

    print("Removing downloaded archive")
    remove(tar_path)


@task
def download_toolchain(ctx):
    tar_name = _get_toolchain_tar_name()
    tar_path = _get_toolchain_tar_path()

    if exists(TOOLCHAIN_INSTALL):
        print("Deleting existing toolchain at {}".format(TOOLCHAIN_INSTALL))
        check_output("rm -rf {}".format(TOOLCHAIN_INSTALL), shell=True)

    if not exists(FAASM_LOCAL_DIR):
        makedirs(FAASM_LOCAL_DIR)

    print("Downloading toolchain archive")
    download_tar_from_s3(MISC_S3_BUCKET, tar_name, FAASM_LOCAL_DIR, boto=False)

    print("Removing downloaded archive")
    remove(tar_path)
