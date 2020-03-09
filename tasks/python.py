from os.path import join, exists
from shutil import rmtree
from subprocess import check_output

from invoke import task

import tasks.util.codegen
from tasks.util.env import PYODIDE_INSTALL_DIR, FAASM_RUNTIME_ROOT, PY_RUNTIME_ROOT, PYODIDE_PACKAGES, \
    FAASM_SHARED_ROOT
from tasks.util.files import glob_remove

# TODO - avoid having to hard-code this
_PACKAGES_INCLUDED = {
    "dulwich": {
        "path": "dulwich/build/dulwich-0.19.11/install/lib/python3.7/site-packages/dulwich",
    },
    "genshi": {
        "path": "Genshi/build/Genshi-0.7.2/install/lib/python3.7/site-packages/genshi",
    },
    "numpy": {
        "path": "numpy/build/numpy-1.15.1/install/lib/python3.7/site-packages/numpy",
    },
    "pyperformance": {
        "path": "pyperformance/build/pyperformance-1.0.0/install/lib/python3.7/site-packages/pyperformance",
    },
    "pyaes": {
        "path": "pyaes/build/pyaes-1.6.1/install/lib/python3.7/site-packages/pyaes",
    },
    "pyfaasm": {
        "path": "pyfaasm/build/pyfaasm-*/install/lib/python3.7/site-packages/pyfaasm",
    },
    "six": {
        "path": "six/build/six-1.12.0/install/lib/python3.7/site-packages/six.py"
    }
}


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
def set_up_package(ctx, pkg_name):
    """
    Set up a given Python package
    """
    _do_set_up_python_packages([pkg_name])

    # Run codegen for just this package
    binary = tasks.codegen.find_codegen_shared_lib()
    pkg_install_dir = join(PY_RUNTIME_ROOT, "site-packages", pkg_name)

    if not exists(pkg_install_dir):
        print("Expected to find package installed at {}".format(pkg_install_dir))
        return 1

    check_output("{} {}".format(binary, pkg_install_dir), shell=True)


def _do_set_up_python_packages(package_names):
    for pkg_name in package_names:
        print("\n --------- {} ---------".format(pkg_name))

        pkg_detail = _PACKAGES_INCLUDED[pkg_name]
        pkg_dir = join(PYODIDE_PACKAGES, pkg_detail["path"])

        # Put files in place
        print("Copying {} into place".format(pkg_name))
        runtime_site_packages = join(PY_RUNTIME_ROOT, "site-packages")
        check_output("cp -r {} {}".format(pkg_dir, runtime_site_packages), shell=True)


@task
def set_up_runtime(ctx):
    """
    Set up the Python runtime
    """
    print("Clearing out pyc files")
    _clear_pyc_files(PYODIDE_INSTALL_DIR)
    _clear_pyc_files(PYODIDE_PACKAGES)

    print("\nRemoving any existing runtime files")
    _remove_runtime_dir("include")
    _remove_runtime_dir("lib")

    print("\nRemoving any existing python functions")
    funcs_dir = join(FAASM_SHARED_ROOT, "pyfuncs")
    if exists(funcs_dir):
        rmtree(funcs_dir)

    print("\nPutting CPython libraries in place")
    check_output("cp -r {}/* {}".format(PYODIDE_INSTALL_DIR, FAASM_RUNTIME_ROOT), shell=True)

    # Set up files in runtime root
    runtime_site_packages = join(PY_RUNTIME_ROOT, "site-packages")
    check_output("mkdir -p {}".format(runtime_site_packages), shell=True)

    # Create a tmp directory for work
    check_output("mkdir -p {}".format(join(PY_RUNTIME_ROOT, "tmp")), shell=True)

    print("\nSetting up packages")
    _do_set_up_python_packages(_PACKAGES_INCLUDED.keys())

    # Run codegen
    codegen(ctx)


@task
def codegen(ctx):
    """
    Run Python codegen
    """
    binary = tasks.util.codegen.find_codegen_shared_lib()
    check_output("{} {}".format(binary, PY_RUNTIME_ROOT), shell=True)
