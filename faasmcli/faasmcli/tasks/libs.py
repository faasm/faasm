from os import makedirs
from multiprocessing import cpu_count
from os.path import exists
from os.path import join
from shutil import rmtree
from subprocess import run

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


@task
def toolchain(ctx, clean=False):
    """
    Compile and install all libs crucial to the toolchain
    """
    eigen(ctx)

    faasm(ctx, clean=clean)
    faasmp(ctx, clean=clean)
    faasmpi(ctx, clean=clean)

    fake(ctx, clean=clean)
    pyinit(ctx, clean=clean)
    rust(ctx, clean=clean)


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

    run(build_cmd_str, shell=True, cwd=build_dir, check=True)
    run("ninja", shell=True, cwd=build_dir, check=True)
    run("sudo ninja install", shell=True, cwd=build_dir, check=True)


def _build_faasm_lib(dir_name, clean, verbose, target=None):
    work_dir = join(PROJ_ROOT, dir_name)
    build_dir = join(PROJ_ROOT, "build", dir_name)

    clean_dir(build_dir, clean)

    build_cmd = [
        "cmake",
        "-GNinja",
        "-DFAASM_BUILD_TYPE=wasm",
        "-DCMAKE_BUILD_TYPE=Release",
        "-DCMAKE_TOOLCHAIN_FILE={}".format(FAASM_TOOLCHAIN_FILE),
        work_dir,
    ]

    build_cmd_str = " ".join(build_cmd)
    print(build_cmd_str)

    run(build_cmd_str, shell=True, cwd=build_dir, check=True)

    build_cmd = ["ninja", target if target else ""]

    run(" ".join(build_cmd), shell=True, cwd=build_dir, check=True)
    run("ninja install", shell=True, cwd=build_dir, check=True)


@task
def faasm(ctx, clean=False, verbose=False):
    """
    Compile and install the Faasm library
    """
    _build_faasm_lib("libs/cpp", clean, verbose)


@task
def faasmp(ctx, clean=False, verbose=False):
    """
    Compile and install the Faasm OpenMP library
    """
    _build_faasm_lib("libs/faasmp", clean, verbose)


@task
def faasmpi(ctx, clean=False, verbose=False):
    """
    Compile and install the Faasm MPI library
    """
    # Build the Faabric MPI target
    _build_faasm_lib(
        "third-party/faabric/src/mpi",
        clean,
        verbose,
    )

    # Build the Faasm wasm wrapper
    _build_faasm_lib("libs/faasmpi", clean, verbose)


@task
def rust(ctx, clean=False, verbose=False):
    """
    Install Rust library
    """
    _build_faasm_lib("libs/rust", clean, verbose)


@task
def pyinit(ctx, clean=False, verbose=False):
    """
    Install pyinit library
    """
    _build_faasm_lib("libs/pyinit", clean, verbose)


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

    run(" ".join(build_cmd), shell=True, cwd=build_dir, check=True)
    run("ninja", shell=True, cwd=build_dir, check=True)
    run("ninja install", shell=True, cwd=build_dir, check=True)

    # Copy shared object into place
    sysroot_files = join(
        SYSROOT_INSTALL_PREFIX, "lib", "wasm32-wasi", "libfake*.so"
    )

    runtime_lib_dir = join(FAASM_RUNTIME_ROOT, "lib")
    if not exists(runtime_lib_dir):
        makedirs(runtime_lib_dir)

    run(
        "cp {} {}".format(sysroot_files, runtime_lib_dir),
        shell=True,
        check=True,
    )

    # Run codegen
    shared_objs = [
        join(FAASM_RUNTIME_ROOT, "lib", "libfakeLibA.so"),
        join(FAASM_RUNTIME_ROOT, "lib", "libfakeLibB.so"),
    ]

    binary = find_codegen_shared_lib()

    for so in shared_objs:
        print("Running codegen for {}".format(so))
        run("{} {}".format(binary, so), shell=True, check=True)


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

    run(cmd_string, shell=True, cwd=build_dir, check=True)

    run(
        "{} make install".format(verbose_string),
        shell=True,
        cwd=build_dir,
        check=True,
    )


@task
def clapack(ctx, clean=False):
    # See the CLAPACK docs: http://www.netlib.org/clapack/
    proj_dir = join(THIRD_PARTY_DIR, "faasm-clapack")
    clapack_dir = join(proj_dir, "third-party", "clapack")
    lapacke_dir = join(proj_dir, "third-party", "lapack", "LAPACKE")

    # Build clapack
    if clean:
        run("make clean", cwd=clapack_dir, shell=True, check=True)
        run("make clean", cwd=lapacke_dir, shell=True, check=True)

    n_cpu = int(cpu_count()) - 1

    # Make libf2c first (needed by others)
    run(
        "make f2clib -j {}".format(n_cpu),
        shell=True,
        cwd=clapack_dir,
        check=True,
    )

    # Make the rest of CLAPACK
    run("make -j {}".format(n_cpu), shell=True, cwd=clapack_dir, check=True)
    run("make install", shell=True, cwd=clapack_dir, check=True)

    # Make LAPACKE
    run("make -j {}".format(n_cpu), shell=True, cwd=lapacke_dir, check=True)
    run("make install", shell=True, cwd=lapacke_dir, check=True)

