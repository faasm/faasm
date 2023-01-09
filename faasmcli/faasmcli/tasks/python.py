from copy import copy
from os import environ
from os.path import join, exists
from shutil import rmtree
from subprocess import run

from invoke import task

from faasmcli.util import codegen as cg
from faasmcli.util.env import (
    FAASM_RUNTIME_ROOT,
    PY_RUNTIME_ROOT,
)
from faasmcli.util.files import glob_remove


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
def clear_runtime_pyc(ctx):
    """
    Clear out runtime .pyc files
    """
    print("Clearing out runtime pyc files")
    _clear_pyc_files(FAASM_RUNTIME_ROOT)


@task
def codegen(ctx, clean=False):
    """
    Run Python codegen
    """
    # Update env
    shell_env = copy(environ)
    shell_env.update(
        {
            "LD_LIBRARY_PATH": "/usr/local/lib/",
        }
    )

    binary = cg.find_codegen_shared_lib()
    codegen_cmd = [
        binary,
        PY_RUNTIME_ROOT,
        "--clean" if clean else "",
    ]
    codegen_cmd = " ".join(codegen_cmd)
    print(codegen_cmd)
    run(
        codegen_cmd,
        env=shell_env,
        shell=True,
        check=True,
    )
