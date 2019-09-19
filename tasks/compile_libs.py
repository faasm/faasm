from os import mkdir
from os.path import exists
from os.path import join
from shutil import rmtree
from subprocess import call, check_output

from invoke import task

from tasks.compile import clean_dir
from tasks.util.codegen import find_codegen_binary
from tasks.util.env import PROJ_ROOT, FAASM_TOOLCHAIN_FILE, FAASM_SYSROOT, FAASM_INSTALL_DIR, \
    FAASM_RUNTIME_ROOT, THIRD_PARTY_DIR


@task
def compile_malloc(ctx, clean=False):
    work_dir = join(PROJ_ROOT, "third-party", "malloc")
    build_dir = join(work_dir, "build")

    clean_dir(build_dir, clean)

    build_cmd = [
        "cmake",
        "-DCMAKE_BUILD_TYPE=Release",
        "-DCMAKE_TOOLCHAIN_FILE={}".format(FAASM_TOOLCHAIN_FILE),
        ".."
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
        ".."
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
            ".."
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
    build_dir = join(work_dir, "build")

    clean_dir(build_dir, clean)

    build_cmd = [
        "cmake",
        "-DFAASM_BUILD_TYPE=wasm",
        "-DCMAKE_TOOLCHAIN_FILE={}".format(FAASM_TOOLCHAIN_FILE),
        "-DCMAKE_BUILD_TYPE=Release",
        "-DCMAKE_INSTALL_PREFIX={}".format(FAASM_SYSROOT),
        ".."
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

    binary = find_codegen_binary()

    for so in shared_objs:
        print("Running codegen for {}".format(so))
        check_output("{} {}".format(binary, so), shell=True)


@task
def compile_eigen(ctx):
    eigen_build_dir = join(THIRD_PARTY_DIR, "eigen", "build")

    if exists(eigen_build_dir):
        rmtree(eigen_build_dir)

    mkdir(eigen_build_dir)
    cmd = [
        "cmake",
        "-DFAASM_BUILD_TYPE=Release",
        "-DCMAKE_TOOLCHAIN_FILE={}".format(FAASM_TOOLCHAIN_FILE),
        "-DCMAKE_BUILD_TYPE=Release",
        "-DCMAKE_INSTALL_PREFIX={}".format(FAASM_SYSROOT),
        ".."
    ]
    cmd_string = " ".join(cmd)

    call(cmd_string, shell=True, cwd=eigen_build_dir)
    call("make install", shell=True, cwd=eigen_build_dir)

#
# @task
# def compile_gemmlowp(ctx):
#     # gemmlowp is header-only
#     working_dir = "/tmp"
#     gemm_dir = "/tmp/gemmlowp"
#     dest_dir = join(FAASM_SYSROOT, "include", "gemmlowp")
#
#     # These are the dirs from the gemmlowp repo to include
#     include_dirs = [
#         "meta",
#         "public",
#         "profiling",
#         "fixedpoint",
#         "internal",
#     ]
#
#     if not exists(gemm_dir):
#         print("Checkout gemmlowp")
#         call("git clone https://github.com/google/gemmlowp.git gemmlowp", cwd=working_dir, shell=True)
#
#     if not exists(dest_dir):
#         mkdir(dest_dir)
#
#     for dir_name in include_dirs:
#         dir_path = join(gemm_dir, dir_name)
#         dest_path = join(dest_dir, dir_name)
#
#         if exists(dest_path):
#             print("Deleting existing {}".format(dest_path))
#             rmtree(dest_path)
#
#         print("Copying {} to {}".format(dir_path, dest_path))
#         call("cp -r {} {}".format(dir_path, dest_path), shell=True)
