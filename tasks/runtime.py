import glob
from os import remove
from os.path import join, exists
from shutil import rmtree
from subprocess import check_output

from invoke import task

from tasks.env import PROJ_ROOT, PYODIDE_INSTALL_DIR, FAASM_RUNTIME_ROOT, PY_RUNTIME_ROOT, PYODIDE_PACKAGES, \
    FAASM_LOCAL_DIR, MISC_S3_BUCKET
from tasks.upload_util import upload_file_to_s3, download_file_from_s3

RUNTIME_TAR_NAME = "faasm_runtime_root.tar.gz"
RUNTIME_TAR_PATH = "/tmp/{}".format(RUNTIME_TAR_NAME)

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


def _glob_remove(glob_pattern, recursive=False, directory=False):
    print("Recursive remove: {}".format(glob_pattern))
    for filename in glob.iglob(glob_pattern, recursive=recursive):
        print("Removing {}".format(filename))
        if directory:
            rmtree(filename)
        else:
            remove(filename)


def _clear_pyc_files(dir_path):
    pyc_glob = "{}/**/*.pyc".format(dir_path)
    _glob_remove(pyc_glob, recursive=True)

    pycache_glob = "{}/**/__pycache__".format(dir_path)
    _glob_remove(pycache_glob, recursive=True, directory=True)


def _remove_runtime_dir(dir_name):
    dir_path = join(FAASM_RUNTIME_ROOT, dir_name)
    if exists(dir_path):
        rmtree(dir_path)


@task
def clear_runtime_pyc(ctx):
    print("Clearing out runtime pyc files")
    _clear_pyc_files(FAASM_RUNTIME_ROOT)


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

    print("\nSetting up packages")
    for pkg_name, pkg_detail in _PACKAGES_INCLUDED.items():
        print("\n --------- {} ---------".format(pkg_name))

        pkg_dir = join(PYODIDE_PACKAGES, pkg_detail["path"])

        # Put files in place
        print("Copying {} into place".format(pkg_name))
        check_output("cp -r {} {}".format(pkg_dir, runtime_site_packages), shell=True)

    # Run codegen
    run_python_codegen(ctx)


@task
def package_python_runtime(ctx):
    # Clear out existing object files
    print("Removing any existing platform-specific files")
    obj_glob = "{}/**/*.o".format(FAASM_RUNTIME_ROOT)
    _glob_remove(obj_glob, recursive=True)

    # Compress
    print("Creating archive of faasm runtime root")
    check_output("tar -cf {} runtime_root".format(RUNTIME_TAR_PATH), shell=True, cwd=FAASM_LOCAL_DIR)

    # Upload
    print("Uploading archive to S3")
    upload_file_to_s3(RUNTIME_TAR_PATH, MISC_S3_BUCKET, RUNTIME_TAR_NAME)

    # Remove old tar
    print("Removing archive")
    check_output("rm {}".format(RUNTIME_TAR_PATH))


@task
def download_python_runtime(ctx):
    # Clear out existing
    print("Removing existing")
    rmtree(FAASM_RUNTIME_ROOT)

    # Download the bundle
    print("Downloading from S3")
    downloaded_tar_path = "/usr/local/faasm/{}".format(RUNTIME_TAR_NAME)
    download_file_from_s3(MISC_S3_BUCKET, RUNTIME_TAR_NAME, downloaded_tar_path)

    # Extract
    print("Extracting")
    check_output("tar -xf {}".format(RUNTIME_TAR_NAME), shell=True, cwd=FAASM_LOCAL_DIR)

    # Run codegen
    print("Running codegen")
    run_python_codegen(ctx)


@task
def run_python_codegen(ctx):
    print("Running codegen for {}".format(PY_RUNTIME_ROOT))

    possible_binaries = [
        "cmake-build-release/bin/codegen",
        "cmake-build-debug/bin/codegen",
        "build/bin/codegen",
    ]
    possible_binaries = [join(PROJ_ROOT, p) for p in possible_binaries]

    existing_binaries = [p for p in possible_binaries if exists(p)]
    if not existing_binaries:
        print("Could not find any codegen binaries (options = {})".format(possible_binaries))
        exit(1)

    binary = existing_binaries[0]
    if len(existing_binaries) > 1:
        print("WARNING: found multiple codegen binaries, taking {}".format(binary))

    check_output("{} {}".format(binary, PY_RUNTIME_ROOT), shell=True)
