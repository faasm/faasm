from invoke import task
from os.path import join, exists
from shutil import rmtree
from tasks.util.env import FAASM_RUNTIME_ROOT
from tasks.util.files import glob_remove


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
