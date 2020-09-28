from os.path import join, exists
from shutil import rmtree, copytree
from subprocess import run
from os import makedirs

from invoke import task

from faasmcli.util import codegen as cg
from faasmcli.util.env import (
    FAASM_RUNTIME_ROOT,
    THIRD_PARTY_DIR,
    PY_RUNTIME_ROOT,
)
from faasmcli.util.files import glob_remove


FAASM_CPYTHON_DIR = join(THIRD_PARTY_DIR, "faasm-cpython")
CPYTHON_SRC = join(FAASM_CPYTHON_DIR, "third-party", "cpython")
CPYTHON_INSTALL_DIR = join(CPYTHON_SRC, "install", "wasm")
CROSSENV_WASM_DIR = join(FAASM_CPYTHON_DIR, "cross_venv", "cross")


def _clear_pyc_files(dir_path):
    pyc_glob = "{}/**/*.pyc".format(dir_path)
    glob_remove(pyc_glob, recursive=True)

    pycache_glob = "{}/**/__pycache__".format(dir_path)
    glob_remove(pycache_glob, recursive=True, directory=True)


def _remove_runtime_dir(dir_name):
    dir_path = join(FAASM_RUNTIME_ROOT, dir_name)
    if exists(dir_path):
        rmtree(dir_path)


@task
def copy_cpython(ctx):
    """
    Copies the CPython archive and all installed modules from the Faasm cpython
    build into the Faasm runtime root
    """

    include_root = join(FAASM_RUNTIME_ROOT, "include")
    lib_root = join(FAASM_RUNTIME_ROOT, "lib")

    if not exists(lib_root):
        print("Creating {}".format(lib_root))
        makedirs(lib_root)

    if not exists(include_root):
        print("Creating {}".format(include_root))
        makedirs(include_root)

    include_src_dir = join(CPYTHON_INSTALL_DIR, "include", "python3.8")
    lib_src_dir = join(CPYTHON_INSTALL_DIR, "lib", "python3.8")
    site_packages_src_dir = join(
        CROSSENV_WASM_DIR, "lib", "python3.8", "site-packages"
    )

    include_dest_dir = join(include_root, "python3.8")
    lib_dest_dir = join(lib_root, "python3.8")
    site_packages_dest_dir = join(lib_dest_dir, "site-packages")

    # Clear out pyc files
    print("Clearing out pyc files")
    _clear_pyc_files(lib_src_dir)
    _clear_pyc_files(site_packages_src_dir)

    # Remove dirs to be replaced by those we copy in
    if exists(include_dest_dir):
        print("Removing {}".format(include_dest_dir))
        rmtree(include_dest_dir)

    if exists(lib_dest_dir):
        print("Removing {}".format(lib_dest_dir))
        rmtree(lib_dest_dir)

    # Copy CPython includes
    print("Copying {} to {}".format(include_src_dir, include_dest_dir))
    copytree(include_src_dir, include_dest_dir)

    # Copy CPython libs
    print("Copying {} to {}".format(lib_src_dir, lib_dest_dir))
    copytree(lib_src_dir, lib_dest_dir)

    # Copy cross-compiled modules
    if not exists(site_packages_dest_dir):
        makedirs(site_packages_dest_dir)

    print(
        "Copying {} to {}".format(
            site_packages_src_dir, site_packages_dest_dir
        )
    )
    run(
        "cp -r {}/* {}/".format(site_packages_src_dir, site_packages_dest_dir),
        shell=True,
        check=True,
    )


@task
def clear_runtime_pyc(ctx):
    """
    Clear out runtime .pyc files
    """
    print("Clearing out runtime pyc files")
    _clear_pyc_files(FAASM_RUNTIME_ROOT)


@task
def runtime(ctx):
    """
    Set up the Python runtime
    """
    print("Copying the CPython runtime files into place")
    copy_cpython(ctx)

    print("\nRemoving any existing python functions")
    funcs_dir = join(FAASM_RUNTIME_ROOT, "pyfuncs")
    if exists(funcs_dir):
        rmtree(funcs_dir)

    # Create a tmp directory
    run(
        "mkdir -p {}".format(join(PY_RUNTIME_ROOT, "tmp")),
        shell=True,
        check=True,
    )

    # Run codegen
    codegen(ctx)


@task
def codegen(ctx):
    """
    Run Python codegen
    """
    binary = cg.find_codegen_shared_lib()
    run("{} {}".format(binary, PY_RUNTIME_ROOT), shell=True, check=True)
