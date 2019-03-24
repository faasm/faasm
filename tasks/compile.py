from os import mkdir, environ
from os.path import exists
from os.path import join
from shutil import rmtree
from subprocess import call

from invoke import task

from tasks.download import download_proj
from tasks.env import PROJ_ROOT, SYSROOT, PY_EMSCRIPTEN_CMAKE_TOOLCHAIN, PY_EMSCRIPTEN_DIR, WASM_TOOLCHAIN


def check_correct_emscripten(expected_root):
    actual_root = environ.get("EMSCRIPTEN")

    if not actual_root or not actual_root.startswith(expected_root):
        path_parts = expected_root.split("/")
        emsdk_path = path_parts[:-2]
        emsdk_path.append("emsdk_env.sh")

        print("You are not running the expected emscripten. Start a new shell and run: ")
        print("source {}".format("/".join(emsdk_path)))
        exit(1)


@task
def clean_build(context):
    lib_build_dir = join(PROJ_ROOT, "lib", "wasm_lib_build")

    # Clean build of library
    rmtree(lib_build_dir)
    compile_libfaasm(context)

    # Clean build of functions
    funcs(context, clean=True)


@task
def funcs_emscripten(context, clean=False, func=None, debug=False):
    _build_funcs("emscripten", clean=clean, func=func, cmake_build_type="Debug" if debug else "Release")


@task
def funcs_python(context, clean=False, func=None, debug=False):
    _build_funcs("python", clean=clean, func=func, cmake_build_type="Debug" if debug else "Release")


@task
def funcs(context, clean=False, func=None):
    _build_funcs("wasm", clean=clean, func=func)


def _get_toolchain(build_type):
    if build_type in ["emscripten", "python"]:
        check_correct_emscripten(PY_EMSCRIPTEN_DIR)
        toolchain_file = PY_EMSCRIPTEN_CMAKE_TOOLCHAIN
    else:
        toolchain_file = None

    return toolchain_file


def _build_funcs(build_type, clean=False, func=None, cmake_build_type="Release"):
    """
    Compiles functions
    """

    toolchain_file = _get_toolchain(build_type)
    func_build_dir = join(PROJ_ROOT, "func", "{}_func_build".format(build_type))

    if clean and exists(func_build_dir):
        rmtree(func_build_dir)

    if not exists(func_build_dir):
        mkdir(func_build_dir)

    build_cmd = [
        "cmake",
        "-DFAASM_BUILD_TYPE={}".format(build_type),
        "-DCMAKE_TOOLCHAIN_FILE={}".format(toolchain_file) if toolchain_file else "",
        "-DCMAKE_BUILD_TYPE={}".format(cmake_build_type),
        ".."
    ]

    call(" ".join(build_cmd), shell=True, cwd=func_build_dir)

    # Allow specifying a single function
    if func:
        cmd = "make VERBOSE=1 {}".format(func)
    else:
        cmd = "make"

    call(cmd, shell=True, cwd=func_build_dir)


@task
def compile_libfaasm_python(ctx):
    _do_libfaasm_build("python")


@task
def compile_libfaasm_emscripten(ctx):
    _do_libfaasm_build("emscripten")


@task
def compile_libfaasm(ctx):
    _do_libfaasm_build("wasm")


def _do_libfaasm_build(build_type, cmake_build_type="Release"):
    work_dir = join(PROJ_ROOT, "lib")
    build_dir = join(work_dir, "{}_lib_build".format(build_type))

    if exists(build_dir):
        rmtree(build_dir)

    mkdir(build_dir)

    build_cmd = [
        "cmake",
        "-DFAASM_BUILD_TYPE={}".format(build_type),
        "-DCMAKE_BUILD_TYPE={}".format(cmake_build_type),
        "-DCMAKE_TOOLCHAIN_FILE={}".format(WASM_TOOLCHAIN),
        ".."
    ]

    build_cmd_str = " ".join(build_cmd)
    print(build_cmd_str)

    call(build_cmd_str, shell=True, cwd=build_dir)
    call("make VERBOSE=1", shell=True, cwd=build_dir)
    call("make install", shell=True, cwd=build_dir)

    # Put imports file in place to avoid undefined symbols
    call("cp libfaasm.imports {}".format(build_dir), shell=True, cwd=work_dir)


@task
def compile_libfake(ctx):
    work_dir = join(PROJ_ROOT, "func", "libfake")

    build_dir = join(work_dir, "build")
    if exists(build_dir):
        rmtree(build_dir)
    mkdir(build_dir)

    toolchain_file = _get_toolchain("python")

    build_cmd = [
        "cmake",
        "-DCMAKE_TOOLCHAIN_FILE={}".format(toolchain_file) if toolchain_file else "",
        "-DCMAKE_BUILD_TYPE=Release",
        ".."
    ]

    call(" ".join(build_cmd), shell=True, cwd=build_dir)
    call("make VERBOSE=1", shell=True, cwd=build_dir)
    call("make install", shell=True, cwd=build_dir)


def _checkout_eigen():
    extract_dir, build_dir = download_proj(
        "http://bitbucket.org/eigen/eigen/get/3.3.7.tar.gz",
        "3.3.7",
        extract_file="eigen-eigen-323c052e1731"
    )

    if exists(build_dir):
        rmtree(build_dir)

    mkdir(build_dir)

    return build_dir


@task
def compile_eigen_emscripten(ctx):
    build_dir = _checkout_eigen()

    toolchain_file = _get_toolchain("emscripten")

    call("emconfigure cmake -DCMAKE_TOOLCHAIN_FILE={} ..".format(toolchain_file), shell=True, cwd=build_dir)

    call("make", shell=True, cwd=build_dir)
    call("make install", shell=True, cwd=build_dir)


@task
def compile_eigen(ctx):
    build_dir = _checkout_eigen()

    call("cmake -DCMAKE_INSTALL_PREFIX={} ..".format(SYSROOT), shell=True, cwd=build_dir)

    call("make", shell=True, cwd=build_dir)
    call("make install", shell=True, cwd=build_dir)
