from os import makedirs
from os.path import exists
from os.path import join
from shutil import rmtree
from subprocess import check_output, call

from invoke import task

from faasmcli.util.codegen import find_codegen_shared_lib
from faasmcli.util.env import (
    PROJ_ROOT,
    FAASM_TOOLCHAIN_FILE,
    SYSROOT_INSTALL_PREFIX,
    FAASM_INSTALL_DIR,
    FAASM_RUNTIME_ROOT,
)
from faasmcli.util.env import THIRD_PARTY_DIR
from faasmcli.util.files import clean_dir
from faasmcli.util.toolchain import (
    WASM_HOST,
    BASE_CONFIG_CMD,
)
from faasmcli.util.toolchain import WASM_SYSROOT, WASM_BUILD, BASE_CONFIG_FLAGS


@task
def toolchain(ctx, clean=False):
    """
    Compile and install all libs crucial to the toolchain
    """
    eigen(ctx)
    faasm(ctx, clean=clean)
    fake(ctx, clean=clean)


@task
def native(ctx, clean=False):
    """
    Compile and install Faasm native tools
    """
    if not exists(FAASM_INSTALL_DIR):
        makedirs(FAASM_INSTALL_DIR)

    build_dir = join(PROJ_ROOT, "build", "native_tools")
    clean_dir(build_dir, clean)

    build_cmd = [
        "cmake",
        "-GNinja",
        "-DCMAKE_CXX_COMPILER=/usr/bin/clang++-10",
        "-DCMAKE_C_COMPILER=/usr/bin/clang-10",
        "-DFAASM_BUILD_TYPE=native-tools",
        "-DFAASM_STATIC_LIBS=OFF",
        "-DFAABRIC_BUILD_TESTS=OFF",
        "-DCMAKE_BUILD_TYPE=Release",
        "-DCMAKE_INSTALL_PREFIX={}".format(FAASM_INSTALL_DIR),
        PROJ_ROOT,
    ]

    build_cmd_str = " ".join(build_cmd)
    print(build_cmd_str)

    res = call(build_cmd_str, shell=True, cwd=build_dir)
    if res != 0:
        raise RuntimeError("Failed to build native tools")

    res = call("ninja", shell=True, cwd=build_dir)
    if res != 0:
        raise RuntimeError("Failed to make native tools")

    call("sudo ninja install", shell=True, cwd=build_dir)


def _build_faasm_lib(dir_name, clean, verbose):
    work_dir = join(PROJ_ROOT, "libs", dir_name)
    build_dir = join(PROJ_ROOT, "build", dir_name)

    clean_dir(build_dir, clean)

    verbose_str = "VERBOSE=1" if verbose else ""
    build_cmd = [
        verbose_str,
        "cmake",
        "-GNinja",
        "-DFAASM_BUILD_TYPE=wasm",
        "-DCMAKE_BUILD_TYPE=Release",
        "-DCMAKE_TOOLCHAIN_FILE={}".format(FAASM_TOOLCHAIN_FILE),
        work_dir,
    ]

    build_cmd_str = " ".join(build_cmd)
    print(build_cmd_str)

    res = call(build_cmd_str, shell=True, cwd=build_dir)
    if res != 0:
        exit(1)

    res = call("{} ninja".format(verbose_str), shell=True, cwd=build_dir)
    if res != 0:
        exit(1)

    res = call("ninja install", shell=True, cwd=build_dir)
    if res != 0:
        exit(1)


@task
def faasm(ctx, clean=False, lib=None, verbose=False):
    """
    Compile and install all Faasm libraries
    """
    if lib:
        _build_faasm_lib(lib, clean, verbose)
    else:
        _build_faasm_lib("cpp", clean, verbose)
        _build_faasm_lib("pyinit", clean, verbose)
        _build_faasm_lib("faasmp", clean, verbose)
        _build_faasm_lib("rust", clean, verbose)


@task
def faasmp(ctx, clean=False, verbose=False):
    """
    Compile and install the Faasm OpenMP library
    """
    _build_faasm_lib("faasmp", clean, verbose)


@task
def rust(ctx, clean=False, verbose=False):
    """
    Install Rust library
    """
    _build_faasm_lib("rust", clean, verbose)


@task
def fake(ctx, clean=False):
    """
    Compile and install the fake library used for testing
    """
    work_dir = join(PROJ_ROOT, "func", "dynlink")
    build_dir = join(PROJ_ROOT, "build", "libfake")

    clean_dir(build_dir, clean)

    build_cmd = [
        "cmake",
        "-GNinja",
        "-DFAASM_BUILD_TYPE=wasm",
        "-DCMAKE_TOOLCHAIN_FILE={}".format(FAASM_TOOLCHAIN_FILE),
        "-DCMAKE_BUILD_TYPE=Release",
        "-DCMAKE_INSTALL_PREFIX={}".format(SYSROOT_INSTALL_PREFIX),
        work_dir,
    ]

    call(" ".join(build_cmd), shell=True, cwd=build_dir)
    call("ninja VERBOSE=1 ", shell=True, cwd=build_dir)
    call("ninja install", shell=True, cwd=build_dir)

    # Copy shared object into place
    sysroot_files = join(SYSROOT_INSTALL_PREFIX, "lib", "wasm32-wasi", "libfake*.so")

    runtime_lib_dir = join(FAASM_RUNTIME_ROOT, "lib")
    if not exists(runtime_lib_dir):
        makedirs(runtime_lib_dir)

    call("cp {} {}".format(sysroot_files, runtime_lib_dir), shell=True)

    # Run codegen
    shared_objs = [
        join(FAASM_RUNTIME_ROOT, "lib", "libfakeLibA.so"),
        join(FAASM_RUNTIME_ROOT, "lib", "libfakeLibB.so"),
    ]

    binary = find_codegen_shared_lib()

    for so in shared_objs:
        print("Running codegen for {}".format(so))
        check_output("{} {}".format(binary, so), shell=True)


@task
def eigen(ctx, verbose=False):
    """
    Compile and install Eigen
    """
    work_dir = join(THIRD_PARTY_DIR, "eigen")
    build_dir = join(PROJ_ROOT, "build", "eigen")

    if exists(build_dir):
        rmtree(build_dir)
    makedirs(build_dir)

    verbose_string = "VERBOSE=1" if verbose else ""

    cmd = [
        verbose_string,
        "cmake",
        "-DFAASM_BUILD_TYPE=wasm",
        "-DCMAKE_TOOLCHAIN_FILE={}".format(FAASM_TOOLCHAIN_FILE),
        "-DCMAKE_BUILD_TYPE=Release",
        "-DCMAKE_INSTALL_PREFIX={}".format(SYSROOT_INSTALL_PREFIX),
        work_dir,
    ]
    cmd_string = " ".join(cmd)

    res = call(cmd_string, shell=True, cwd=build_dir)
    if res != 0:
        exit(1)

    res = call("{} make install".format(verbose_string), shell=True, cwd=build_dir)
    if res != 0:
        exit(1)


@task
def png(ctx):
    """
    Compile and install libpng
    """
    workdir = join(PROJ_ROOT, "third-party", "libpng")

    config_cmd = BASE_CONFIG_CMD
    config_cmd.extend(BASE_CONFIG_FLAGS)
    config_cmd.extend(
        [
            "./configure",
            "--build={}".format(WASM_BUILD),
            "--host={}".format(WASM_HOST),
            "--prefix={}".format(WASM_SYSROOT),
        ]
    )

    check_output(" ".join(config_cmd), shell=True, cwd=workdir)
    check_output("make", shell=True, cwd=workdir)
    check_output("make install", shell=True, cwd=workdir)


@task
def zlib(ctx, clean=False):
    """
    Compile and install zlib
    """
    workdir = join(PROJ_ROOT, "third-party", "zlib")

    config_cmd = BASE_CONFIG_CMD
    config_cmd.extend(BASE_CONFIG_FLAGS)
    config_cmd.extend(
        [
            "./configure",
            "--static",
            "--prefix={}".format(WASM_SYSROOT),
        ]
    )

    check_output(" ".join(config_cmd), shell=True, cwd=workdir)
    check_output("make", shell=True, cwd=workdir)
    check_output("make install", shell=True, cwd=workdir)

