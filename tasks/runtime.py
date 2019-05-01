import glob
from os import remove
from os.path import join, exists
from shutil import rmtree
from subprocess import check_output

from invoke import task

from tasks.env import PROJ_ROOT, PYODIDE_INSTALL_DIR, FAASM_RUNTIME_ROOT, PY_RUNTIME_ROOT, PYODIDE_PACKAGES

# TODO - avoid having to hard-code this
_PACKAGES_INCLUDED = {
    "numpy": {
        "path": "numpy/build/numpy-1.15.1/install/lib/python3.7/site-packages/numpy",
    },
    "perf": {
        "path": "perf/build/perf-1.6.0/install//lib/python3.7/site-packages/perf"
    },
    "performance": {
        "path": "performance/build/performance-0.7.0/install/lib/python3.7/site-packages/performance",
    },
    "six": {
        "path": "six/build/six-1.12.0/install/lib/python3.7/site-packages/six.py"
    }
}


def _glob_remove(glob_pattern, recursive=False):
    print("Recursive remove: {}".format(glob_pattern))
    for filename in glob.iglob(glob_pattern, recursive=recursive):
        print("Removing {}".format(filename))
        remove(filename)


def _clear_pyc_files(dir_path):
    pycache_glob = "{}/**/__pycache__".format(dir_path)
    pyc_glob = "{}/**/*.pyc".format(dir_path)

    _glob_remove(pyc_glob, recursive=True)
    _glob_remove(pycache_glob, recursive=True)


def _remove_runtime_dir(dir_name):
    dir_path = join(FAASM_RUNTIME_ROOT, dir_name)
    if exists(dir_path):
        rmtree(dir_path)


@task
def set_up_python_runtime(ctx):
    print("Clearing out pyc files")
    _clear_pyc_files(PYODIDE_INSTALL_DIR)
    _clear_pyc_files(PYODIDE_PACKAGES)

    print("\nRemoving any existing runtime files")
    _remove_runtime_dir("funcs")
    _remove_runtime_dir("include")
    _remove_runtime_dir("lib")

    print("\nPutting CPython libraries in place")
    check_output("cp -r {}/* {}".format(PYODIDE_INSTALL_DIR, FAASM_RUNTIME_ROOT), shell=True)

    print("\nPutting python functions in place")
    funcs_dir = join(PROJ_ROOT, "python", "funcs")
    check_output("cp -r {} {}".format(funcs_dir, FAASM_RUNTIME_ROOT), shell=True)

    # Set up files in runtime root
    runtime_site_packages = join(PY_RUNTIME_ROOT, "site-packages")
    check_output("mkdir -p {}".format(runtime_site_packages), shell=True)

    print("\nCopying packages")
    for pkg_name, pkg_detail in _PACKAGES_INCLUDED.items():
        print("Copying {} into place".format(pkg_name))

        pkg_dir = join(PYODIDE_PACKAGES, pkg_detail["path"])
        check_output("cp -r {} {}".format(pkg_dir, runtime_site_packages), shell=True)
