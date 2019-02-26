import copy
import os
from os import mkdir
from os.path import exists
from os.path import join
from shutil import rmtree
from subprocess import call

from invoke import task

from tasks.download import download_proj, FAASM_HOME, clone_proj
from tasks.env import PROJ_ROOT

# Note, most of the time this will be run inside the toolchain container
TOOLCHAIN_ROOT = "/toolchain"

WASM_LIB_DIR = join(PROJ_ROOT, "wasm", "lib")

SYSROOT = join(TOOLCHAIN_ROOT, "sysroot")

TARGET_TRIPLE = "wasm32-unknown-unknown-wasm"
CONFIG_TARGET = "wasm32"
CONFIG_HOST = "wasm32-unknown-none"

COMPILER_FLAGS = [
    "--target={}".format(TARGET_TRIPLE),
    "--sysroot={}".format(SYSROOT),
]

COMPILER_FLAGS_STRING = " ".join(COMPILER_FLAGS)
COMPILER_FLAGS_STRING = "\"{}\"".format(COMPILER_FLAGS_STRING)

# Getting everything mapped properly here is crucial. If we end up
# using system defaults we can get some errors that are *very* hard
# to debug

NATIVE_CC = "/usr/bin/clang"
NATIVE_CPP = "/usr/bin/clang-cpp"
NATIVE_CFLAGS = ""
NATIVE_CXX = "/usr/bin/clang++"
NATIVE_CXXFLAGS = ""
NATIVE_LD = "/usr/bin/ld"
NATIVE_CROSS_COMPILE = ""
NATIVE_AR = "/usr/bin/ar"
NATIVE_AS = "/usr/bin/as"
NATIVE_RANLIB = "/usr/bin/ranlib"

CC = join(TOOLCHAIN_ROOT, "bin", "clang")
CPP = join(TOOLCHAIN_ROOT, "bin", "clang-cpp")
CFLAGS = COMPILER_FLAGS_STRING
CXX = join(TOOLCHAIN_ROOT, "bin", "clang++")
CXXFLAGS = COMPILER_FLAGS_STRING
LD = join(TOOLCHAIN_ROOT, "bin", "wasm-ld")
CROSS_COMPILE = join(TOOLCHAIN_ROOT, "llvm-")
AR = join(TOOLCHAIN_ROOT, "bin", "llvm-ar")
AS = join(TOOLCHAIN_ROOT, "bin", "llvm-as")
RANLIB = join(TOOLCHAIN_ROOT, "bin", "llvm-ranlib")

_ENV_TUPLES = [
    ("CC", CC),
    ("CPP", CPP),
    ("CFLAGS", CFLAGS),
    ("AR", AR),
    ("AS", AS),
    ("LD", LD),
    ("RANLIB", RANLIB),
    ("CROSS_COMPILE", CROSS_COMPILE),
    ("CXX", CXX),
    ("CXXFLAGS", CXXFLAGS),
]

_NATIVE_ENV_TUPLES = [
    ("CC", NATIVE_CC),
    ("CPP", NATIVE_CPP),
    ("CFLAGS", NATIVE_CFLAGS),
    ("AR", NATIVE_AR),
    ("AS", NATIVE_AS),
    ("LD", NATIVE_LD),
    ("RANLIB", NATIVE_RANLIB),
    ("CROSS_COMPILE", NATIVE_CROSS_COMPILE),
    ("CXX", NATIVE_CXX),
    ("CXXFLAGS", NATIVE_CXXFLAGS),
]

ENV_DICT = {e[0]: e[1] for e in _ENV_TUPLES}
ENV_STR = " ".join(["{}={}".format(e[0], e[1]) for e in _ENV_TUPLES])

NATIVE_ENV_DICT = {e[0]: e[1] for e in _NATIVE_ENV_TUPLES}
NATIVE_ENV_STR = " ".join(["{}={}".format(e[0], e[1]) for e in _NATIVE_ENV_TUPLES])

EMSCRIPTEN_DIR = "/usr/local/code/emsdk/emscripten/1.38.27/"
EMSCRIPTEN_CMAKE_TOOLCHAIN = join(EMSCRIPTEN_DIR, "cmake", "Modules", "Platform", "Emscripten.cmake")
INITIAL_PATH = os.environ["PATH"]
EMSCRIPTEN_ENV_DICT = {
    "PATH": "{}:{}".format(INITIAL_PATH, EMSCRIPTEN_DIR)
}
EMSCRIPTEN_SYSROOT = join(FAASM_HOME, "emsysroot")

BUILD_DIR = join(PROJ_ROOT, "work")

CONFIG_FLAGS = [
    "--target={}".format(CONFIG_TARGET),
    "--host={}".format(CONFIG_TARGET),
    "--prefix={}".format(SYSROOT),
]


@task
def clean_build(context):
    lib_build_dir = join(PROJ_ROOT, "lib", "build")

    # Clean build of library
    rmtree(lib_build_dir)
    compile_libfaasm(context)

    # Clean build of functions
    funcs(context, clean=True)


@task
def funcs_emscripten(context, clean=False, func=None):
    _build_funcs("emscripten", clean=clean, func=func)


@task
def funcs(context, clean=False, func=None):
    _build_funcs("wasm", clean=clean, func=func)


def _build_funcs(build_type, clean=False, func=None, toolchain_file=None, top_level_build=False):
    """
    Compiles functions
    """

    if build_type == "emscripten":
        toolchain_file = EMSCRIPTEN_CMAKE_TOOLCHAIN

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
def build_python_emscripten(ctx):
    proj_dir = clone_proj("https://github.com/Shillaker/cpython-emscripten", "cpython-emscripten")

    # Make sure emscripten is present
    assert exists(EMSCRIPTEN_DIR), "Must have emscripten ready"

    # Run the emscripten python build
    current_path = os.environ["PATH"]
    make_dir = join(proj_dir, "3.5.2")
    call("make", cwd=make_dir, shell=True, env=EMSCRIPTEN_ENV_DICT)


@task
def build_python_host(ctx):
    # -----------------------------------
    # Build for native host (used by cross-compile build steps)
    # -----------------------------------

    host_proj = clone_proj("https://github.com/Shillaker/cpython", "cpython-host", branch="wasm")
    host_build_dir = join(host_proj, "host_build")

    call("./configure --prefix={}".format(host_build_dir),
         shell=True, cwd=host_proj, env=NATIVE_ENV_DICT)

    # call("make regen-grammar", shell=True, cwd=host_proj, env=NATIVE_ENV_DICT)
    call("make python Parser/pgen", shell=True, cwd=host_proj, env=NATIVE_ENV_DICT)
    call("make install", shell=True, cwd=host_proj, env=NATIVE_ENV_DICT)


@task
def build_python(ctx):
    # Note, full python version is 3.6.7
    host_proj = join(FAASM_HOME, "cpython-host")
    host_python = join(host_proj, "host_build", "bin", "python3")
    host_pgen = join(host_proj, "Parser", "pgen")
    python_lib = "libpython3.6.a"

    # -----------------------------------
    # Configure cross-compile
    # -----------------------------------
    target_proj = clone_proj("https://github.com/Shillaker/cpython", "cpython-wasm", branch="wasm")

    config_cmd = [
        "CONFIG_SITE=./config.site",
        "READELF=true",
        "./configure",
        ENV_STR,
        "--without-threads",
        "--without-pymalloc",
        "--disable-shared",
        "--disable-ipv6",
        "--without-gcc",
        "--target={}".format(CONFIG_TARGET),
        "--host={}".format(CONFIG_HOST),
        "--build={}".format(CONFIG_TARGET),
        "--prefix={}".format(SYSROOT),
    ]

    # Run configuration
    config_cmd_str = " ".join(config_cmd)
    call(config_cmd_str, shell=True, cwd=target_proj)

    # Copy setup local file into place
    call("cp wasm32/Modules_Setup.local Modules/Setup.local", shell=True, cwd=target_proj)

    # -----------------------------------
    # Build static zlib
    # -----------------------------------

    zlib_config_cmd = [
        ENV_STR,
        "./configure",
        "--static",
    ]
    zlib_config_cmd_str = " ".join(zlib_config_cmd)
    call(zlib_config_cmd_str, cwd=join(target_proj, "Modules", "zlib"), shell=True)

    base_make_cmd = [
        "make",
        "HOSTPYTHON={}".format(host_python),
        "HOSTPGEN={}".format(host_pgen),
        "CROSS_COMPILE=yes",
    ]

    # -----------------------------------
    # Build core Python lib
    # -----------------------------------
    make_cmd_a = copy.copy(base_make_cmd)
    make_cmd_a.append(python_lib)

    # Invoke the make command
    call(" ".join(make_cmd_a), shell=True, cwd=target_proj)

    # -----------------------------------
    # Build all Python standard modules
    # -----------------------------------

    # Remove any of the dependencies of the libinstall target
    call("sed -i -e 's/libinstall:.*/libinstall:/' Makefile;", shell=True, cwd=target_proj)

    # Build make command
    make_cmd_b = copy.copy(base_make_cmd)
    make_cmd_b.extend([
        "PYTHON_FOR_BUILD={}".format(host_python),
        "inclinstall",
        python_lib,
        "libinstall"
    ])

    call(" ".join(make_cmd_b), shell=True, cwd=target_proj)

    # -----------------------------------
    # Tidy up
    # -----------------------------------

    # Put lib in place
    call("cp {} {}".format(python_lib, join(SYSROOT, "lib")), shell=True, cwd=target_proj)

    # Put sysconfig in place
    # host_sysconfig = join(host_build_dir, "lib", "python3.6", "_sysconfigdata_m_linux_x86_64-linux-gnu.py")
    # sysconfig_dest = join(SYSROOT, "lib", "python3.6", "_sysconfigdata__wasm32_x86_64-linux-gnu.py")
    # os.copy(host_sysconfig, sysconfig_dest)


@task
def compile_libfaasm_emscripten(ctx):
    work_dir = join(PROJ_ROOT, "lib")
    build_dir = join(work_dir, "embuild")

    if exists(build_dir):
        rmtree(build_dir)

    mkdir(build_dir)

    call("emconfigure cmake -DFAASM_BUILD_TYPE=wasm -DCMAKE_TOOLCHAIN_FILE={} ..".format(EMSCRIPTEN_CMAKE_TOOLCHAIN),
         shell=True, cwd=build_dir, env=EMSCRIPTEN_ENV_DICT)
    call("make", shell=True, cwd=build_dir, env=EMSCRIPTEN_ENV_DICT)

    # Put imports file in place to avoid undefined symbols
    call("cp libfaasm.imports {}".format(build_dir), shell=True, cwd=work_dir)


@task
def compile_libfaasm(ctx):
    if not exists(FAASM_HOME):
        mkdir(FAASM_HOME)

    work_dir = join(PROJ_ROOT, "lib")
    build_dir = join(work_dir, "build")

    if exists(build_dir):
        rmtree(build_dir)

    mkdir(build_dir)

    build_cmd = "{} cmake -DFAASM_BUILD_TYPE=wasm ..".format(ENV_STR)
    print(build_cmd)
    call(build_cmd, shell=True, cwd=build_dir)

    call("make", shell=True, cwd=build_dir)
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

    call("emconfigure cmake -DCMAKE_TOOLCHAIN_FILE={} ..".format(EMSCRIPTEN_CMAKE_TOOLCHAIN),
         shell=True, cwd=build_dir, env=EMSCRIPTEN_ENV_DICT)

    call("make", shell=True, cwd=build_dir, env=EMSCRIPTEN_ENV_DICT)
    call("make install", shell=True, cwd=build_dir, env=EMSCRIPTEN_ENV_DICT)


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
