from os import mkdir
from os.path import exists
from os.path import join
from shutil import rmtree
from subprocess import call, check_output

from invoke import task

from tasks.util.codegen import find_codegen_binary
from tasks.util.env import PROJ_ROOT, FAASM_TOOLCHAIN_FILE, FAASM_SYSROOT, FUNC_BUILD_DIR, FAASM_INSTALL_DIR, \
    FAASM_RUNTIME_ROOT, LATEST_CMAKE


def _clean_dir(dir_path, clean):
    if clean and exists(dir_path):
        rmtree(dir_path)

    if not exists(dir_path):
        mkdir(dir_path)


@task
def compile(context, clean=False, func=None, debug=False, user=None):
    build_type = "wasm"
    cmake_build_type = "Debug" if debug else "Release"

    _clean_dir(FUNC_BUILD_DIR, clean)

    build_cmd = [
        "cmake",
        "-DFAASM_BUILD_TYPE={}".format(build_type),
        "-DCMAKE_TOOLCHAIN_FILE={}".format(FAASM_TOOLCHAIN_FILE),
        "-DCMAKE_BUILD_TYPE={}".format(cmake_build_type),
        ".."
    ]

    res = call(" ".join(build_cmd), shell=True, cwd=FUNC_BUILD_DIR)
    if res != 0:
        print("Failed to compile")
        return

    # Allow specifying a single function
    target = None
    if func:
        target = func
    elif user:
        target = "{}_all_funcs".format(user)

    cmd = "make {}".format(target) if target else "make -j"
    call(cmd, shell=True, cwd=FUNC_BUILD_DIR)


@task
def compile_malloc(ctx, clean=False):
    work_dir = join(PROJ_ROOT, "malloc")
    build_dir = join(work_dir, "build")

    _clean_dir(build_dir, clean)

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

    build_dir = join(PROJ_ROOT, "native_build")
    _clean_dir(build_dir, clean)

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
        build_dir = join(work_dir, "lib_build")

        _clean_dir(build_dir, clean)

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

    _clean_dir(build_dir, clean)

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
    working_dir = "/tmp"
    eigen_dir = join(working_dir, "eigen3")
    eigen_build_dir = join(eigen_dir, "build")

    if not exists(eigen_dir):
        print("Checkout eigen")
        call("git clone git@github.com:eigenteam/eigen-git-mirror.git eigen3",
             shell=True, cwd=working_dir)

        # Make sure this commit hash matches the one in the Ansible task
        print("Checkout specific eigen commit")
        call("git checkout 0bdcefe7257e0a7c328c8440b85617e4ad75f3cf",
             shell=True, cwd=eigen_dir)

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


@task
def compile_onnx(ctx, clean=False):
    _do_compile_onnx_runtime(False)


@task
def compile_onnx_native(ctx):
    _do_compile_onnx_runtime(True)


def _do_compile_onnx_runtime(native):
    work_dir = join(PROJ_ROOT, "onnxruntime")

    if native:
        build_dir = join(work_dir, "build", "native")
        eigen_path = "/usr/local/include/eigen3"
    else:
        build_dir = join(work_dir, "build", "wasm")
        eigen_path = join(FAASM_SYSROOT, "include", "eigen3")

    # _clean_dir(build_dir, clean)

    # See the script itself for more info on options (onnxruntime/tools/ci_build/build.py)
    # More docs in the repo too: https://github.com/microsoft/onnxruntime/blob/master/BUILD.md
    script_args = [
        "--cmake_path={}".format(LATEST_CMAKE),
        "--use_preinstalled_eigen",
        "--eigen_path={}".format(eigen_path),
        "--build_dir={}".format(build_dir),
        "--skip_onnx_tests",
        "--parallel",
    ]

    # Wasm-specific
    if not native:
        script_args.extend([
            "--wasm",
        ])

    # Check if we've already cloned the onnxruntime subprojects (and skip if so)
    cmake_submodule_checkout = join(PROJ_ROOT, "onnxruntime", "cmake", "external", "protobuf", "cmake")
    if exists(cmake_submodule_checkout):
        script_args.append("--skip_submodule_sync")
    else:
        call("git submodule update --init --recursive", cwd=work_dir)

    # Run the main build
    build_type = "RelWithDebInfo"
    build_cmd = [
        "python3", "tools/ci_build/build.py",
        "--update",
        "--build",
        "--build_shared_lib",
        "--config={}".format(build_type)
    ]
    build_cmd.extend(script_args)
    build_cmd_str = " ".join(build_cmd)
    print(build_cmd_str)
    res = call(build_cmd_str, shell=True, cwd=work_dir)
    if res != 0:
        print("Build command failed")
        exit(1)

    # Do the install
    make_dir = join(build_dir, build_type)
    call("make install", cwd=make_dir, shell=True)
