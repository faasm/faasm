from os import mkdir
from os.path import exists
from os.path import join
from shutil import rmtree, copyfile
from subprocess import call

from invoke import task
from requests import get

from tasks.env import PROJ_ROOT, get_wasm_func_path

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
DOWNLOAD_DIR = join(PROJ_ROOT, "download")

CONFIG_FLAGS = [
    "--target={}".format(CONFIG_TARGET),
    "--host={}".format(CONFIG_TARGET),
    "--prefix={}".format(SYSROOT),
]


@task
def compile(context, user, func_name, libcurl=False, debug=False, cxx=False):
    """
    Compiles the given function
    """

    print("Compiling {} for user {}".format(func_name, user))

    # Recreate the build dir
    if exists(BUILD_DIR):
        rmtree(BUILD_DIR)
    mkdir(BUILD_DIR)

    func_path = join("func", "function_{}.c".format(func_name))

    output_file = join("work", "out.wasm")

    compile_cmd = [
        CXX if cxx else CC,
        *COMPILER_FLAGS,
        "-Oz",
        "-fvisibility=hidden",
        func_path,
        "-I", join("include", "faasm"),
        "-o", output_file
    ]

    # Debug
    if debug:
        compile_cmd.append("-g")

    # Add libcurl
    if libcurl:
        compile_cmd.append("-lcurl")

    compile_cmd_str = " ".join(compile_cmd)
    res = call(compile_cmd_str, shell=True, cwd=PROJ_ROOT)
    if res != 0:
        raise RuntimeError("Compile call failed")

@task
def lib(context, lib_name):
    if not exists(DOWNLOAD_DIR):
        mkdir(DOWNLOAD_DIR)

    if lib_name == "curl":
        compile_libcurl()
    else:
        raise RuntimeError("Unrecognised lib name: {}".format(lib_name))


def compile_libcurl():
    """
    Compiles libcurl to wasm
    """

    LIBCURL_URL = "https://github.com/curl/curl/archive/curl-7_61_0.tar.gz"
    CURL_EXTRACT_DIR = join(DOWNLOAD_DIR, "curl-curl-7_61_0")

    if not exists(CURL_EXTRACT_DIR):
        # Download the file
        with open(join(DOWNLOAD_DIR, "libcurl.tar.gz"), "wb") as fh:
            response = get(LIBCURL_URL)
            fh.write(response.content)

        # Extract
        call(["tar", "-xvf", "libcurl.tar.gz"], cwd=DOWNLOAD_DIR)

    # Buildconf
    buildconf_cmd = ["./buildconf"]
    res = call(buildconf_cmd, shell=True, cwd=CURL_EXTRACT_DIR)
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
    res = call(config_cmd_str, shell=True, cwd=CURL_EXTRACT_DIR)
    if res != 0:
        raise RuntimeError("Configure command failed")

    # Make
    make_cmd = ["make"]
    res = call(make_cmd, shell=True, cwd=CURL_EXTRACT_DIR)
    if res != 0:
        raise RuntimeError("Make failed")

    res = call("make install", shell=True, cwd=CURL_EXTRACT_DIR)
    if res != 0:
        raise RuntimeError("Make install failed")
