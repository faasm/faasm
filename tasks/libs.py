from os import makedirs, mkdir
from os.path import exists
from os.path import join
from shutil import rmtree, copy
from subprocess import check_output, call

from invoke import task

from tasks.compile import clean_dir
from tasks.util.codegen import find_codegen_shared_lib
from tasks.util.env import PROJ_ROOT, FAASM_TOOLCHAIN_FILE, SYSROOT_INSTALL_PREFIX, FAASM_INSTALL_DIR, \
    FAASM_RUNTIME_ROOT, WASM_DIR
from tasks.util.env import THIRD_PARTY_DIR
from toolchain.python_env import WASM_HOST, BASE_CONFIG_CMD, WASM_CFLAGS, WASM_CXXFLAGS, WASM_LDFLAGS, WASM_CC, \
    WASM_CXX, WASM_RANLIB, WASM_AR, WASM_LD
from toolchain.python_env import WASM_SYSROOT, WASM_BUILD, \
    BASE_CONFIG_FLAGS

PRK_DIR = join(THIRD_PARTY_DIR, "ParResKernels")


@task
def toolchain(ctx, clean=False):
    """
    Compile and install all libs crucial to the toolchain
    """
    eigen(ctx)
    faasm(ctx, clean=clean)
    fake(ctx, clean=clean)
    # zlib(ctx)
    # tflite(ctx, clean=clean)


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
        "-DFAASM_BUILD_TYPE=native-tools",
        "-DFAASM_AWS_SUPPORT=OFF",
        "-DFAASM_STATIC_LIBS=OFF",
        "-DCMAKE_BUILD_TYPE=Release",
        "-DCMAKE_INSTALL_PREFIX={}".format(FAASM_INSTALL_DIR),
        PROJ_ROOT,
    ]

    build_cmd_str = " ".join(build_cmd)
    print(build_cmd_str)

    res = call(build_cmd_str, shell=True, cwd=build_dir)
    if res != 0:
        raise RuntimeError("Failed to build native tools")

    res = call("make -j", shell=True, cwd=build_dir)
    if res != 0:
        raise RuntimeError("Failed to make native tools")

    call("sudo make install", shell=True, cwd=build_dir)


def _build_faasm_lib(dir_name, clean, verbose):
    work_dir = join(PROJ_ROOT, "libs", dir_name)
    build_dir = join(PROJ_ROOT, "build", dir_name)

    clean_dir(build_dir, clean)

    verbose_str = "VERBOSE=1" if verbose else ""
    build_cmd = [
        verbose_str,
        "cmake",
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

    res = call("{} make".format(verbose_str), shell=True, cwd=build_dir)
    if res != 0:
        exit(1)

    res = call("make install", shell=True, cwd=build_dir)
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
        _build_faasm_lib("faasmpi", clean, verbose)
        _build_faasm_lib("rust", clean, verbose)


@task
def faasmp(ctx, clean=False, verbose=False):
    """
    Compile and install the Faasm OpenMP library
    """
    _build_faasm_lib("faasmp", clean, verbose)


@task
def faasmpi(ctx, clean=False, verbose=False):
    """
    Compile and install the Faasm MPI library
    """
    _build_faasm_lib("faasmpi", clean, verbose)


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
        "-DFAASM_BUILD_TYPE=wasm",
        "-DCMAKE_TOOLCHAIN_FILE={}".format(FAASM_TOOLCHAIN_FILE),
        "-DCMAKE_BUILD_TYPE=Release",
        "-DCMAKE_INSTALL_PREFIX={}".format(SYSROOT_INSTALL_PREFIX),
        work_dir,
    ]

    call(" ".join(build_cmd), shell=True, cwd=build_dir)
    call("make VERBOSE=1 ", shell=True, cwd=build_dir)
    call("make install", shell=True, cwd=build_dir)

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
def lulesh(ctx, mpi=False, omp=False, clean=True, debug=False, cp=True):
    """
    Compile and install the LULESH code
    """
    work_dir = join(THIRD_PARTY_DIR, "LULESH")

    if omp and mpi:
        build_dir = "ompi"
        target = "lulesh_ompi"
    elif omp:
        build_dir = "omp-only"
        target = "lulesh_omp"
    elif mpi:
        build_dir = "mpi-only"
        target = "lulesh_mpi"
    else:
        build_dir = "bare"
        target = "lulesh_bare"
    build_dir = join(work_dir, "build", build_dir)

    if clean and exists(build_dir):
        print("rm {}".format(build_dir))
        rmtree(build_dir)

    if not exists(build_dir):
        makedirs(build_dir)

    cmd = " ".join([
        "cmake",
        "-G Ninja",
        "-DWITH_MPI={}".format("TRUE" if mpi else "FALSE"),
        "-DWITH_OPENMP={}".format("TRUE" if omp else "FALSE"),
        "-DWITH_SILO=FALSE",
        "-DFAASM_BUILD_TYPE=wasm",
        "-DCMAKE_TOOLCHAIN_FILE={}".format(FAASM_TOOLCHAIN_FILE),
        "-DCMAKE_BUILD_TYPE=Release",
        "-DCMAKE_INSTALL_PREFIX={}".format(SYSROOT_INSTALL_PREFIX),
        work_dir
    ])
    if debug:
        print("Running {}".format(cmd))
    res = call(cmd, shell=True, cwd=build_dir)
    if res != 0:
        raise RuntimeError("Failed on cmake init for {}".format(target))

    cmd = " ".join([
        "cmake",
        "--build {}".format(build_dir),
        "--target {}".format(target),
    ])
    if debug:
        print("Running {}".format(cmd))
    res = call(cmd, shell=True)
    if res != 0:
        raise RuntimeError("Failed on make for {}".format(target))

    if cp:
        dest_dir = join(WASM_DIR, "lulesh", target)
        if clean and exists(dest_dir):
            rmtree(dest_dir)

        if not exists(dest_dir):
            makedirs(dest_dir)

        cmd = " ".join([
            "cp",
            "{}.wasm".format(target),
            join(dest_dir, "function.wasm"),
        ])
        if debug:
            print("Running {}".format(cmd))
        res = call(cmd, shell=True, cwd=build_dir)
        if res != 0:
            raise RuntimeError("Failed to copy {}".format(target))


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
        work_dir
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
    config_cmd.extend([
        "./configure",
        "--build={}".format(WASM_BUILD),
        "--host={}".format(WASM_HOST),
        "--prefix={}".format(WASM_SYSROOT),
    ])

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
    config_cmd.extend([
        "./configure",
        "--static",
        "--prefix={}".format(WASM_SYSROOT),
    ])

    check_output(" ".join(config_cmd), shell=True, cwd=workdir)
    check_output("make", shell=True, cwd=workdir)
    check_output("make install", shell=True, cwd=workdir)


@task
def tflite(ctx, clean=False):
    """
    Compile and install Tensorflow Lite
    """
    tf_dir = join(THIRD_PARTY_DIR, "tensorflow")
    tf_lite_dir = join(tf_dir, "tensorflow", "lite")
    tf_make_dir = join(tf_lite_dir, "tools", "make")

    download_check_dir = join(tf_make_dir, "downloads", "absl")
    if not exists(download_check_dir):
        download_script = join(tf_make_dir, "download_dependencies.sh")
        check_output(download_script, shell=True)

    make_target = "lib"
    make_cmd = ["make -j 4"]
    make_cmd.extend(BASE_CONFIG_CMD)
    make_cmd.extend([
        "CFLAGS=\"{} -ftls-model=local-exec\"".format(WASM_CFLAGS),
        "CXXFLAGS=\"{}\"".format(WASM_CXXFLAGS),
        "LDFLAGS=\"{} -Xlinker --max-memory=4294967296\"".format(WASM_LDFLAGS),
        "MINIMAL_SRCS=",
        "TARGET={}".format(WASM_HOST),
        "BUILD_WITH_MMAP=false",
        "LIBS=\"-lstdc++\"",
        "-C \"{}\"".format(tf_dir),
        "-f tensorflow/lite/tools/make/Makefile",
    ])

    make_cmd.append(make_target)

    clean_dir = join(tf_make_dir, "gen", "wasm32-unknown-none_x86_64")
    if clean and exists(clean_dir):
        rmtree(clean_dir)

    res = call(" ".join(make_cmd), shell=True, cwd=tf_lite_dir)
    if res != 0:
        raise RuntimeError("Failed to compile Tensorflow lite")


@task
def prk(ctx, clean=False):
    """
    Compile and install the ParRes Kernels
    """

    # Pairs of (directory, make target). See top-level ParResKernels
    # Makefile for more examples
    make_targets = [
        ("MPI1/Synch_global", "global"),
        ("MPI1/Synch_p2p", "p2p"),
        ("MPI1/Sparse", "sparse"),
        ("MPI1/Transpose", "transpose"),
        ("MPI1/Stencil", "stencil"),
        ("MPI1/DGEMM", "dgemm"),
        ("MPI1/Nstream", "nstream"),
        ("MPI1/Reduce", "reduce"),
        ("MPI1/Random", "random"),
        # ("MPI1/PIC-static", "pic"),
        # ("MPI1/AMR", "amr"),
    ]

    if clean:
        call("make clean", shell=True, cwd=PRK_DIR)

    has_failed = False
    for subdir, make_target in make_targets:
        make_cmd = "make {}".format(make_target)
        make_dir = join(PRK_DIR, subdir)
        res = call(make_cmd, shell=True, cwd=make_dir)

        if res != 0:
            print("Making kernel in {} with target {} failed".format(subdir, make_target))
            has_failed = True

        print("Built kernel {}".format(make_target))

    if has_failed:
        print("At least one kernel failed")
        exit(1)

    # Make sure wasm dir exists
    prk_wasm_dest = join(WASM_DIR, "prk")
    if not exists(prk_wasm_dest):
        mkdir(prk_wasm_dest)

    # Copy the functions into place
    prk_wasm_src = join(PRK_DIR, "wasm")
    for target in [t[1] for t in make_targets]:
        wasm_src = join(prk_wasm_src, "{}.wasm".format(target))
        wasm_dest = join(prk_wasm_dest, target)
        if not exists(wasm_dest):
            mkdir(wasm_dest)

        copy(wasm_src, join(wasm_dest, "function.wasm"))


@task
def mpi_bench(ctx, clean=False):
    """
    Compile and install the Intel MPI benchmarks
    """
    bench_dir = join(THIRD_PARTY_DIR, "mpi-benchmarks")
    make_target = "IMB-MPI1"

    if clean:
        call("make clean", shell=True, cwd=bench_dir)

    make_cmd = "make -j 4 {}".format(make_target)
    res = call(make_cmd, shell=True, cwd=bench_dir, env={
        "CC": WASM_CC,
        "CXX": WASM_CXX,
        "CFLAGS": WASM_CFLAGS,
        "CXXFLAGS": WASM_CXXFLAGS,
        "AR": WASM_AR,
        "RANLIB": WASM_RANLIB,
        "LD": WASM_LD,
        "LDFLAGS": "{} -lmpi".format(WASM_LDFLAGS),
    })

    if res != 0:
        raise RuntimeError("Failed to compile MPI benchmarks")
