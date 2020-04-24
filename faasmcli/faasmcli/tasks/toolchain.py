from os import makedirs
from os import remove
from os.path import exists, join
from shutil import rmtree
from subprocess import check_output, call

from invoke import task

from . import python as this_python
from faasmcli.util.env import FAASM_RUNTIME_ROOT, FAASM_LOCAL_DIR
from faasmcli.util.env import FAASM_SYSROOT
from faasmcli.util.release import get_runtime_tar_name, get_runtime_tar_path, \
    get_sysroot_tar_name, get_toolchain_tar_name, get_toolchain_tar_path, get_sysroot_tar_path, get_toolchain_url, \
    get_runtime_url, get_sysroot_url
from faasmcli.util.upload_util import download_tar_from_url

TOOLCHAIN_INSTALL = join(FAASM_LOCAL_DIR, "toolchain")


@task
def download_sysroot(ctx):
    """
    Download the sysroot for the Faasm toolchain
    """
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
def download_toolchain(ctx, version=None):
    """
    Download the Faasm toolchain
    """
    url = get_toolchain_url()
    tar_name = get_toolchain_tar_name(version=version)
    tar_path = get_toolchain_tar_path(version=version)

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
def download_runtime(ctx, nocodegen=False):
    """
    Download the Faasm runtime files
    """
    url = get_runtime_url()
    tar_name = get_runtime_tar_name()
    tar_path = get_runtime_tar_path()

    # Clear out existing
    if exists(FAASM_RUNTIME_ROOT):
        print("Removing existing")
        rmtree(FAASM_RUNTIME_ROOT)

    # Download the bundle
    print("Downloading")
    download_tar_from_url(url, tar_name, FAASM_LOCAL_DIR)

    # Remove downloaded tar
    remove(tar_path)

    # Run codegen
    if not nocodegen:
        print("Running codegen")
        this_python.codegen(ctx)


@task
def version(ctx):
    """
    Prints the version of various LLVM tools
    """
    bin_dir = join(TOOLCHAIN_INSTALL, "bin")

    for exe in ["clang", "clang++", "llvm-ar", "wasm-ld"]:
        bin_path = join(bin_dir, exe)
        print("---- {} ----".format(exe))
        call("{} --version".format(bin_path), shell=True)
        print("")


def _scp_dir_from(user, host, dir_name):
    _do_scp(True, user, host, dir_name)


def _scp_dir_to(user, host, dir_name):
    _do_scp(False, user, host, dir_name)


def _do_scp(is_from, user, host, dir_name):
    local_dir = join(FAASM_LOCAL_DIR, dir_name)
    if is_from:
        from_dir = "{}@{}:{}".format(user, host, local_dir)
        to_dir = local_dir

        # Nuke the current
        print("Removing existing {}".format(to_dir))
        rmtree(to_dir, ignore_errors=True)
    else:
        from_dir = local_dir

        # Note with the "to" case, we've not nuked the remote file,
        # so the remote dir is one level higher
        to_dir = "{}@{}:{}".format(user, host, FAASM_LOCAL_DIR)

    scp_cmd = [
        "rsync", "-av",
        from_dir,
        to_dir
    ]
    scp_cmd = " ".join(scp_cmd)
    print(scp_cmd)

    res = call(scp_cmd, shell=True)
    if res != 0:
        raise RuntimeError("Failed to copy {}@{}:{}".format(user, host, dir_name))


@task
def scp_toolchain(ctx, user, host):
    """
    Copies the toolchain from a remote host
    """
    _scp_dir_from(user, host, "toolchain")


@task
def scp_sysroot(ctx, user, host):
    """
    Copies the sysroot from a remote host
    """
    _scp_dir_from(user, host, "llvm-sysroot")


@task
def scp_toolchain_to(ctx, user, host):
    """
    Copies the toolchain *to* a remote host
    """
    _scp_dir_to(user, host, "toolchain")


@task
def scp_sysroot_to(ctx, user, host):
    """
    Copies the sysroot *to* a remote host
    """
    _scp_dir_to(user, host, "llvm-sysroot")


@task
def scp_runtime_to(ctx, user, host):
    """
    Copies the runtime root *to* a remote host
    """
    _scp_dir_to(user, host, "runtime_root")
