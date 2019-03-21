from os import mkdir, environ
from os.path import exists
from os.path import join
from shutil import rmtree
from subprocess import call

from invoke import task

from tasks.download import download_proj, FAASM_HOME
from tasks.env import PROJ_ROOT, EMSCRIPTEN_CMAKE_TOOLCHAIN, ENV_STR, SYSROOT, CONFIG_FLAGS, \
    COMPILER_FLAGS, PY_EMSCRIPTEN_CMAKE_TOOLCHAIN, EMSCRIPTEN_DIR, PY_EMSCRIPTEN_DIR, PYODIDE_ROOT


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
    lib_build_dir = join(PROJ_ROOT, "lib", "build")

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
    _build_funcs("pyodide", clean=clean, func=func, cmake_build_type="Debug" if debug else "Release")


@task
def funcs(context, clean=False, func=None):
    _build_funcs("wasm", clean=clean, func=func)


def _get_toolchain(build_type):
    if build_type == "emscripten":
        check_correct_emscripten(EMSCRIPTEN_DIR)
        toolchain_file = EMSCRIPTEN_CMAKE_TOOLCHAIN
    elif build_type == "pyodide":
        check_correct_emscripten(PY_EMSCRIPTEN_DIR)
        toolchain_file = PY_EMSCRIPTEN_CMAKE_TOOLCHAIN
    else:
        toolchain_file = None

    return toolchain_file


def _build_funcs(build_type, clean=False, func=None, top_level_build=False,
                 cmake_build_type="Release"):
    """
    Compiles functions
    """

    toolchain_file = _get_toolchain(build_type)

    if top_level_build:
        func_build_dir = join(PROJ_ROOT, "{}_func_build".format(build_type))
    else:
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
        cmd = "make {}".format(func)
    else:
        cmd = "make"

    call(cmd, shell=True, cwd=func_build_dir)


@task
def compile_libfaasm_python(ctx):
    _do_libfaasm_build("pyodide")


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

    toolchain_file = _get_toolchain(build_type)

    build_cmd = [
        "cmake",
        "-DFAASM_BUILD_TYPE={}".format(build_type),
        "-DCMAKE_TOOLCHAIN_FILE={}".format(toolchain_file) if toolchain_file else "",
        "-DCMAKE_BUILD_TYPE={}".format(cmake_build_type),
        ".."
    ]

    call(" ".join(build_cmd), shell=True, cwd=build_dir)
    call("make VERBOSE=1", shell=True, cwd=build_dir)
    call("make install", shell=True, cwd=build_dir)

    # Put imports file in place to avoid undefined symbols
    call("cp libfaasm.imports {}".format(build_dir), shell=True, cwd=work_dir)


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

    check_correct_emscripten(EMSCRIPTEN_DIR)

    call("emconfigure cmake -DCMAKE_TOOLCHAIN_FILE={} ..".format(EMSCRIPTEN_CMAKE_TOOLCHAIN),
         shell=True, cwd=build_dir)

    call("make", shell=True, cwd=build_dir)
    call("make install", shell=True, cwd=build_dir)


@task
def compile_eigen(ctx):
    build_dir = _checkout_eigen()

    call("cmake -DCMAKE_INSTALL_PREFIX={} ..".format(SYSROOT), shell=True, cwd=build_dir)

    call("make", shell=True, cwd=build_dir)
    call("make install", shell=True, cwd=build_dir)


@task
def compile_libcurl(ctx):
    extract_dir, build_dir = download_proj(
        "https://github.com/curl/curl/archive/curl-7_61_1.tar.gz",
        "curl-curl-7_61_1"
    )

    # Buildconf
    buildconf_cmd = ["./buildconf"]
    res = call(buildconf_cmd, shell=True, cwd=extract_dir)
    if res != 0:
        raise RuntimeError("Buildconf command failed")

    # Configure
    config_cmd = [
        "./configure",
        ENV_STR,
        "--enable-debug",
        "--disable-threaded-resolver",
        "--without-winssl",
        "--without-darwinssl",
    ]
    config_cmd.extend(CONFIG_FLAGS)

    config_cmd_str = " ".join(config_cmd)
    res = call(config_cmd_str, shell=True, cwd=extract_dir)
    if res != 0:
        raise RuntimeError("Configure command failed")

    # Make
    make_cmd = ["make"]
    res = call(make_cmd, shell=True, cwd=extract_dir)
    if res != 0:
        raise RuntimeError("Make failed")

    res = call("make install", shell=True, cwd=extract_dir)
    if res != 0:
        raise RuntimeError("Make install failed")


# ---------------------------------------
# Libraries below not working
# ---------------------------------------

def compile_gsl():
    # TODO gsl doesn't like --host=wasm32
    extract_dir, build_dir = download_proj(
        "ftp://ftp.gnu.org/gnu/gsl/gsl-2.5.tar.gz",
        "gsl-2.5",
    )

    config_cmd = [
        "./configure",
        ENV_STR,
    ]
    config_cmd.extend(COMPILER_FLAGS)
    config_cmd_str = " ".join(config_cmd)

    print(config_cmd_str)
    call(config_cmd_str, shell=True, cwd=extract_dir)


def compile_dlib():
    # TODO dlib requires pthread support
    extract_dir, build_dir = download_proj(
        "http://dlib.net/files/dlib-19.16.tar.bz2",
        "dlib-19.16"
    )


def compile_mlpack():
    extract_dir, build_dir = download_proj(
        "https://github.com/mlpack/mlpack/archive/mlpack-3.0.3.tar.gz",
        "mlpack-3.0.3",
        extract_file="mlpack-mlpack-3.0.3"
    )

    # Cmake
    cmake_cmd = [
        "cmake",
        "-D", "DEBUG=OFF",
        "-D", "PROFILE=OFF",
        "-D", "ARMA_EXTRA_DEBUG=OFF",
        "-D", "BOOST_ROOT=/usr/include/boost",
        "-D", "BUILD_CLI_EXECUTABLES=OFF",
        "-D", "BUILD_PYTHON_BINDINGS=OFF",
        "-D", "BUILD_TESTS=OFF",
        "-D", "USE_OPENMP=OFF",
        ".."
    ]
    cmake_cmd = " ".join(cmake_cmd)

    # TODO mlpack depends on armadillo which in turn depends on blas, lapack etc.
    # Obviously all the transitive dependencies need to be compiled to wasm which is a big project
    call(cmake_cmd, cwd=build_dir, shell=True)
