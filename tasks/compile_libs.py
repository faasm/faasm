from os import makedirs
from os.path import exists
from os.path import join
from shutil import rmtree
from subprocess import check_output, call

from invoke import task

from tasks.compile import clean_dir
from tasks.util.codegen import find_codegen_shared_lib
from tasks.util.env import PROJ_ROOT, FAASM_TOOLCHAIN_FILE, FAASM_SYSROOT, FAASM_INSTALL_DIR, \
    FAASM_RUNTIME_ROOT, WASM_DIR
from tasks.util.env import THIRD_PARTY_DIR
from toolchain.python_env import WASM_HOST, BASE_CONFIG_CMD, WASM_CFLAGS, WASM_CXXFLAGS, WASM_LDFLAGS, WASM_CC, \
    WASM_CXX, WASM_RANLIB, WASM_AR, WASM_LD
from toolchain.python_env import WASM_SYSROOT, WASM_BUILD, \
    BASE_CONFIG_FLAGS

PRK_DIR = join(THIRD_PARTY_DIR, "ParResKernels")


@task
def compile_libc(ctx):
    check_output("./bin/build_musl.sh", shell=True, cwd=PROJ_ROOT)
    compile_malloc(ctx, clean=True)


@task
def compile_malloc(ctx, clean=False):
    work_dir = join(PROJ_ROOT, "third-party", "malloc")
    build_dir = join(PROJ_ROOT, "build", "malloc")

    clean_dir(build_dir, clean)

    build_cmd = [
        "cmake",
        "-DCMAKE_BUILD_TYPE=Release",
        "-DCMAKE_TOOLCHAIN_FILE={}".format(FAASM_TOOLCHAIN_FILE),
        work_dir,
    ]

    build_cmd_str = " ".join(build_cmd)
    print(build_cmd_str)

    call(build_cmd_str, shell=True, cwd=build_dir)
    call("make", shell=True, cwd=build_dir)
    call("make install", shell=True, cwd=build_dir)


@task
def install_native_tools(ctx, clean=False):
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


def _build_faasm_lib(dir_name, clean):
    work_dir = join(PROJ_ROOT, dir_name)
    build_dir = join(PROJ_ROOT, "build", dir_name)

    clean_dir(build_dir, clean)

    build_cmd = [
        "cmake",
        "-DFAASM_BUILD_TYPE=wasm",
        "-DCMAKE_BUILD_TYPE=Release",
        "-DCMAKE_TOOLCHAIN_FILE={}".format(FAASM_TOOLCHAIN_FILE),
        work_dir,
    ]

    build_cmd_str = " ".join(build_cmd)
    print(build_cmd_str)

    call(build_cmd_str, shell=True, cwd=build_dir)
    call("make", shell=True, cwd=build_dir)
    call("make install", shell=True, cwd=build_dir)


@task
def compile_libfaasm(ctx, clean=False):
    """
    Build all Faasm libraries
    """
    _build_faasm_lib("lib-cpp", clean)
    _build_faasm_lib("lib-pyinit", clean)
    _build_faasm_lib("lib-faasmp", clean)
    _build_faasm_lib("lib-faasmpi", clean)


@task
def compile_libfaasmp(ctx, clean=False):
    """
    Build just the faasm OpenMP library
    """
    _build_faasm_lib("lib-faasmp", clean)


@task
def compile_libfaasmpi(ctx, clean=False):
    """
    Build just the faasm MPI library
    """
    _build_faasm_lib("lib-faasmpi", clean)


@task
def compile_libfake(ctx, clean=False):
    work_dir = join(PROJ_ROOT, "func", "dynlink")
    build_dir = join(PROJ_ROOT, "build", "libfake")

    clean_dir(build_dir, clean)

    build_cmd = [
        "cmake",
        "-DFAASM_BUILD_TYPE=wasm",
        "-DCMAKE_TOOLCHAIN_FILE={}".format(FAASM_TOOLCHAIN_FILE),
        "-DCMAKE_BUILD_TYPE=Release",
        "-DCMAKE_INSTALL_PREFIX={}".format(FAASM_SYSROOT),
        work_dir,
    ]

    call(" ".join(build_cmd), shell=True, cwd=build_dir)
    call("make VERBOSE=1 ", shell=True, cwd=build_dir)
    call("make install", shell=True, cwd=build_dir)

    # Copy shared object into place
    sysroot_files = join(FAASM_SYSROOT, "lib", "libfake*.so")

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
def compile_lulesh(ctx, mpi=False, omp=False, clean=True, debug=False, cp=False):
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
        "-DCMAKE_INSTALL_PREFIX={}".format(FAASM_SYSROOT),
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
            target,
            join(dest_dir, "function.wasm"),
        ])
        if debug:
            print("Running {}".format(cmd))
        res = call(cmd, shell=True, cwd=build_dir)
        if res != 0:
            raise RuntimeError("Failed to copy {}".format(target))


@task
def compile_eigen(ctx):
    work_dir = join(THIRD_PARTY_DIR, "eigen")
    build_dir = join(PROJ_ROOT, "build", "eigen")

    if exists(build_dir):
        rmtree(build_dir)
    makedirs(build_dir)

    cmd = [
        "cmake",
        "-DFAASM_BUILD_TYPE=wasm",
        "-DCMAKE_TOOLCHAIN_FILE={}".format(FAASM_TOOLCHAIN_FILE),
        "-DCMAKE_BUILD_TYPE=Release",
        "-DCMAKE_INSTALL_PREFIX={}".format(FAASM_SYSROOT),
        work_dir
    ]
    cmd_string = " ".join(cmd)

    call(cmd_string, shell=True, cwd=build_dir)
    call("make install", shell=True, cwd=build_dir)


@task
def compile_libpng(ctx):
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
def compile_zlib(ctx):
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
def compile_tflite(ctx, clean=False):
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
        "CFLAGS=\"{} -ftls-model=local-exec -ldlmalloc\"".format(WASM_CFLAGS),
        "CXXFLAGS=\"{} -ldlmalloc\"".format(WASM_CXXFLAGS),
        "LDFLAGS=\"{} -Xlinker --max-memory=4294967296\"".format(WASM_LDFLAGS),
        "MINIMAL_SRCS=",
        "TARGET={}".format(WASM_HOST),
        "BUILD_WITH_MMAP=false",
        "LIBS=\"-lstdc++ -ldlmalloc\"",
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
def compile_prk(ctx, clean=False):
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


@task
def compile_mpi_bench(ctx, clean=False):
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
