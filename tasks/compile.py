from os import mkdir
from os.path import exists
from os.path import join
from shutil import rmtree
from subprocess import call

from invoke import task

from tasks.download import download_proj, DOWNLOAD_DIR
from tasks.env import PROJ_ROOT

# Note, most of the time this will be run inside the toolchain container
TOOLCHAIN_ROOT = "/toolchain"

WASM_LIB_DIR = join(PROJ_ROOT, "wasm", "lib")

SYSROOT = join(TOOLCHAIN_ROOT, "sysroot")

TARGET_TRIPLE = "wasm32-unknown-unknown-wasm"
CONFIG_TARGET = "wasm32"

COMPILER_FLAGS = [
    "--target={}".format(TARGET_TRIPLE),
    "--sysroot={}".format(SYSROOT),
]

COMPILER_FLAGS_STRING = " ".join(COMPILER_FLAGS)
COMPILER_FLAGS_STRING = "\"{}\"".format(COMPILER_FLAGS_STRING)

# Getting everything mapped properly here is crucial. If we end up
# using system defaults we can get some errors that are *very* hard
# to debug

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

ENV_DICT = {e[0]: e[1] for e in _ENV_TUPLES}
ENV_STR = " ".join(["{}={}".format(e[0], e[1]) for e in _ENV_TUPLES])

BUILD_DIR = join(PROJ_ROOT, "work")

CONFIG_FLAGS = [
    "--target={}".format(CONFIG_TARGET),
    "--host={}".format(CONFIG_TARGET),
    "--prefix={}".format(SYSROOT),
]


@task
def funcs(context, clean=False):
    """
    Compiles all the functions in this project
    """
    func_build_dir = join(PROJ_ROOT, "func", "build")

    if clean:
        rmtree(func_build_dir)

    if not exists(func_build_dir):
        mkdir(func_build_dir)

    call("cmake -DCMAKE_BUILD_TYPE=wasm ..", shell=True, cwd=func_build_dir)
    call("make", shell=True, cwd=func_build_dir)


@task
def compile(context, path, libcurl=False, debug=False):
    """
    Compiles the given function
    """

    print("Compiling {}".format(path))

    # Recreate the build dir
    if exists(BUILD_DIR):
        rmtree(BUILD_DIR)
    mkdir(BUILD_DIR)

    output_file = join("work", "out.wasm")

    compile_cmd = [
        CXX,
        *COMPILER_FLAGS,
        "-Oz",
        "-fvisibility=hidden",
        "-lfaasm",
        path,
        "-o", output_file,
    ]

    # Debug
    if debug:
        compile_cmd.append("-g")

    # Add libcurl
    if libcurl:
        compile_cmd.append("-lcurl")

    compile_cmd_str = " ".join(compile_cmd)
    print(compile_cmd_str)

    res = call(compile_cmd_str, shell=True, cwd=PROJ_ROOT)
    if res != 0:
        raise RuntimeError("Compile call failed")


@task
def lib(context, lib_name):
    if not exists(DOWNLOAD_DIR):
        mkdir(DOWNLOAD_DIR)

    if lib_name == "curl":
        compile_libcurl()
    elif lib_name == "eigen":
        compile_eigen()
    elif lib_name == "faasm":
        compile_libfaasm()
    elif lib_name == "mlpack":
        compile_mlpack()
    elif lib_name == "dlib":
        compile_dlib()
    elif lib_name == "gsl":
        compile_gsl()
    else:
        raise RuntimeError("Unrecognised lib name: {}".format(lib_name))


def compile_libfaasm():
    work_dir = join(PROJ_ROOT, "lib")
    build_dir = join(work_dir, "build")

    if exists(build_dir):
        rmtree(build_dir)

    mkdir(build_dir)

    build_cmd = "{} cmake -DCMAKE_BUILD_TYPE=wasm ..".format(ENV_STR)
    print(build_cmd)
    call(build_cmd, shell=True, cwd=build_dir)

    call("make", shell=True, cwd=build_dir)
    call("make install", shell=True, cwd=build_dir)


def compile_eigen():
    extract_dir, build_dir = download_proj(
        "http://bitbucket.org/eigen/eigen/get/3.3.7.tar.gz",
        "3.3.7",
        extract_file="eigen-eigen-323c052e1731"
    )

    call("cmake -DCMAKE_INSTALL_PREFIX={} ..".format(SYSROOT), shell=True, cwd=build_dir)

    call("make", shell=True, cwd=build_dir)
    call("make install", shell=True, cwd=build_dir)


def compile_gsl():
    # TODO gsl doesn't like --host=wasm32
    extract_dir, build_dir = download_proj(
        "ftp://ftp.gnu.org/gnu/gsl/gsl-2.5.tar.gz",
        "gsl-2.5",
    )

    config_cmd = [
        "./configure",
        ENV_STR,
        *CONFIG_FLAGS
    ]
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


def compile_libcurl():
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
        *CONFIG_FLAGS
    ]
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
