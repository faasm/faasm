from os import mkdir, environ
from os.path import exists
from os.path import join
from shutil import rmtree
from subprocess import call, check_output

from invoke import task

from tasks.download import download_proj
from tasks.env import PROJ_ROOT, EMSCRIPTEN_TOOLCHAIN, EMSCRIPTEN_DIR, WASM_SYSROOT, PYODIDE_ROOT, FUNC_BUILD_DIR


def _clean_dir(dir_path, clean):
    if clean and exists(dir_path):
        rmtree(dir_path)

    if not exists(dir_path):
        mkdir(dir_path)


def _check_emscripten():
    actual_root = environ.get("EMSDK")

    if not actual_root or not actual_root.startswith(EMSCRIPTEN_DIR):
        print("You are not running the expected emscripten. Start a new shell and run: ")
        print("source {}".format(join(EMSCRIPTEN_DIR, "emsdk_env.sh")))
        exit(1)


@task
def funcs(context, clean=False, func=None, debug=False):
    _check_emscripten()

    build_type = "emscripten"
    cmake_build_type = "Debug" if debug else "Release"

    _clean_dir(FUNC_BUILD_DIR, clean)

    build_cmd = [
        "VERBOSE=1",
        "cmake",
        "-DFAASM_BUILD_TYPE={}".format(build_type),
        "-DCMAKE_TOOLCHAIN_FILE={}".format(EMSCRIPTEN_TOOLCHAIN),
        "-DCMAKE_BUILD_TYPE={}".format(cmake_build_type),
        ".."
    ]

    res = call(" ".join(build_cmd), shell=True, cwd=FUNC_BUILD_DIR)
    if res != 0:
        print("Failed to compile")
        return

    # Allow specifying a single function
    if func:
        cmd = "make -j VERBOSE=1 {}".format(func)
    else:
        cmd = "make -j"

    call(cmd, shell=True, cwd=FUNC_BUILD_DIR)


@task
def compile_malloc(ctx, clean=False):
    _check_emscripten()

    work_dir = join(PROJ_ROOT, "malloc")
    build_dir = join(work_dir, "emscripten_lib_build")

    _clean_dir(build_dir, clean)

    build_cmd = [
        "cmake",
        "-DCMAKE_BUILD_TYPE=Release",
        "-DCMAKE_TOOLCHAIN_FILE={}".format(EMSCRIPTEN_TOOLCHAIN),
        ".."
    ]

    build_cmd_str = " ".join(build_cmd)
    print(build_cmd_str)

    call(build_cmd_str, shell=True, cwd=build_dir)
    call("make", shell=True, cwd=build_dir)
    call("make install", shell=True, cwd=build_dir)


@task
def compile_libfaasm(ctx, clean=False):
    """
    Build both the main Faasm library and python helper library
    """

    _check_emscripten()

    def _do_lib_build(dir_name):
        work_dir = join(PROJ_ROOT, dir_name)
        build_dir = join(work_dir, "lib_build")

        _clean_dir(build_dir, clean)

        build_cmd = [
            "cmake",
            "-DFAASM_BUILD_TYPE=emscripten",
            "-DCMAKE_BUILD_TYPE=Release",
            "-DCMAKE_TOOLCHAIN_FILE={}".format(EMSCRIPTEN_TOOLCHAIN),
            ".."
        ]

        build_cmd_str = " ".join(build_cmd)
        print(build_cmd_str)

        call(build_cmd_str, shell=True, cwd=build_dir)
        call("make VERBOSE=1", shell=True, cwd=build_dir)
        call("make install", shell=True, cwd=build_dir)

        # Put imports file in place to avoid undefined symbols
        if dir_name == "lib":
            check_output("cp libfaasm.imports {}".format(build_dir), shell=True, cwd=work_dir)

    _do_lib_build("lib")
    _do_lib_build("python")


@task
def compile_libfake(ctx, clean=False):
    _check_emscripten()
    work_dir = join(PROJ_ROOT, "func", "dynlink")

    build_dir = join(work_dir, "build")

    _clean_dir(build_dir, clean)

    build_cmd = [
        "cmake",
        "-DFAASM_BUILD_TYPE=emscripten",
        "-DCMAKE_TOOLCHAIN_FILE={}".format(EMSCRIPTEN_TOOLCHAIN),
        "-DCMAKE_BUILD_TYPE=Release",
        ".."
    ]

    call(" ".join(build_cmd), shell=True, cwd=build_dir)
    call("make VERBOSE=1", shell=True, cwd=build_dir)
    call("make install", shell=True, cwd=build_dir)


@task
def compile_eigen(ctx):
    extract_dir, build_dir = download_proj(
        "http://bitbucket.org/eigen/eigen/get/3.3.7.tar.gz",
        "3.3.7",
        extract_file="eigen-eigen-323c052e1731"
    )

    dest_dir = join(WASM_SYSROOT, "include", "eigen3")
    _clean_dir(dest_dir, True)

    # Eigen is header-only so we just need to copy the files in place
    src_dir = join(extract_dir, "Eigen")
    call("cp -r {} {}".format(src_dir, dest_dir), shell=True)
