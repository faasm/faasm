from os import mkdir, makedirs
from os.path import exists
from os.path import join
from shutil import rmtree
from subprocess import call, check_output

from invoke import task

from tasks.compile import clean_dir
from tasks.util.codegen import find_codegen_shared_lib
from tasks.util.env import PROJ_ROOT, FAASM_TOOLCHAIN_FILE, FAASM_SYSROOT, FAASM_INSTALL_DIR, \
    FAASM_RUNTIME_ROOT, THIRD_PARTY_DIR
from toolchain.python_env import WASM_SYSROOT, WASM_HOST, WASM_BUILD, \
    BASE_CONFIG_CMD, BASE_CONFIG_FLAGS


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
        mkdir(FAASM_INSTALL_DIR)

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

    call(build_cmd_str, shell=True, cwd=build_dir)
    call("make -j", shell=True, cwd=build_dir)
    call("make install", shell=True, cwd=build_dir)


@task
def compile_libfaasm(ctx, clean=False):
    """
    Build all Faasm libraries
    """

    def _do_lib_build(dir_name):
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

        # Put imports file in place to avoid undefined symbols
        if dir_name == "lib":
            check_output("cp libfaasm.imports {}".format(build_dir), shell=True, cwd=work_dir)

    _do_lib_build("lib-cpp")
    _do_lib_build("python")


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

    if not exists(FAASM_RUNTIME_ROOT):
        mkdir(FAASM_RUNTIME_ROOT)

    call("cp {} {}".format(sysroot_files, FAASM_RUNTIME_ROOT), shell=True)

    # Run codegen
    shared_objs = [
        join(FAASM_RUNTIME_ROOT, "libfakeLibA.so"),
        join(FAASM_RUNTIME_ROOT, "libfakeLibB.so"),
    ]

    binary = find_codegen_shared_lib()

    for so in shared_objs:
        print("Running codegen for {}".format(so))
        check_output("{} {}".format(binary, so), shell=True)


@task
def compile_eigen(ctx):
    work_dir = join(THIRD_PARTY_DIR, "eigen")
    build_dir = join(PROJ_ROOT, "build", "eigen")

    if exists(build_dir):
        rmtree(build_dir)
    makedirs(build_dir)

    cmd = [
        "cmake",
        "-DFAASM_BUILD_TYPE=Release",
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
