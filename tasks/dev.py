from faasmtools.build import FAASM_RUNTIME_ENV_DICT, get_dict_as_cmake_vars
from invoke import task
from os import listdir, makedirs
from os.path import exists, getmtime, join
from subprocess import run
from sys import exit
from tasks.util.env import (
    FAASM_BUILD_DIR,
    FAASM_CONAN_CACHE,
    FAASM_CONAN_PROFILES,
    FAASM_INSTALL_DIR,
    FAASM_SGX_MODE_DISABLED,
    FAASM_SGX_MODE_HARDWARE,
    FAASM_SGX_MODE_SIM,
    LLVM_MAJOR_VERSION,
    PROJ_ROOT,
)

DEV_TARGETS = [
    "codegen_func",
    "codegen_shared_obj",
    "func_runner",
    "func_sym",
    "is_app_migratable",
    "local_pool_runner",
    "planner_server",
    "pool_runner",
    "upload",
    "tests",
]

SANITISER_NONE = "None"


def check_build_type(build):
    build_types = ["Debug", "Release"]
    if build not in build_types:
        print(
            f"ERROR: unrecognised build type: {build}. must be one in: {build_types}"
        )
        exit(1)


def get_build_dir(build_type, sgx_mode):
    build_type = build_type.lower()
    build_dir = f"{FAASM_BUILD_DIR}/{build_type}"

    if sgx_mode != FAASM_SGX_MODE_DISABLED:
        if sgx_mode == FAASM_SGX_MODE_SIM:
            build_dir += "-sgx-sim"
        elif sgx_mode == FAASM_SGX_MODE_HARDWARE:
            build_dir += "-sgx-hw"
        else:
            print(f"ERROR: unrecognised sgx mode: {sgx_mode}")
            exit(1)

    return build_dir


def get_current_target_build_dir():
    """
    Infer the correct build dir based on the last modified directory.
    """
    # List all `/build/faasm/debug-*` or `/build/faasm/release-*` directories
    build_dirs = [
        join(FAASM_BUILD_DIR, entry)
        for entry in listdir(FAASM_BUILD_DIR)
        if entry.startswith("debug") or entry.startswith("release")
    ]

    # Pick the one that was modified the latest
    return max(build_dirs, key=getmtime)


def soft_link_bin_dir(build_dir):
    """
    Irrespective of the build type, it is convenient to have the Faasm binaries
    in `/build/faasm/bin`. This makes it possible to hot-patch a binary using
    `faasmctl restart -s <service>` even if we change the build type or SGX
    mode. Thus, after any build, we soft-link `/build/faasm/bin` to whatever
    is the last `bin` directory we have written to. This also prevents
    accidentally using binaries compiled in different modes.
    """
    run(f"ln -sf {build_dir}/bin {FAASM_BUILD_DIR}", shell=True, check=True)


@task
def conan(ctx, clean=False, build="Debug", sanitiser="None"):
    """
    Configure dependencies using Conan
    """
    conan_lockfile = f"{PROJ_ROOT}/conan-{build.lower()}.lock"
    conan_cache = f"{FAASM_CONAN_CACHE}/{build.lower()}"
    if sanitiser == "Thread":
        conan_profile = join(FAASM_CONAN_PROFILES, "tsan.txt")
    elif sanitiser == "Address":
        conan_profile = join(FAASM_CONAN_PROFILES, "asan.txt")
    else:
        conan_profile = join(FAASM_CONAN_PROFILES, "default.txt")

    if clean:
        run(f"rm -f {conan_lockfile}", shell=True, check=True)
        run(f"rm -rf {conan_cache}", shell=True, check=True)

    # Generate a conan lock file if it does not exist. This file should be
    # comitted for version control.
    if not exists(conan_lockfile):
        run("conan remote list", shell=True, check=True)
        conan_cmd = (
            f"conan lock create {PROJ_ROOT} -pr:h={conan_profile} "
            f"-pr:b={conan_profile} -s build_type={build} "
            f"--lockfile-out={conan_lockfile}"
        )
        print(conan_cmd)
        run(conan_cmd, shell=True, check=True)

    # Ensure a clean build by re-building all Conan packages
    build_type = "*" if clean else "missing"

    conan_install_cmd = (
        f"conan install {PROJ_ROOT} -pr:h={conan_profile} "
        f"-pr:b={conan_profile} -s build_type={build} -of {conan_cache} "
        f"--build={build_type} --lockfile={conan_lockfile}"
    )
    print(conan_install_cmd)
    run(conan_install_cmd, shell=True, check=True)


@task
def cmake(
    ctx,
    clean=False,
    build="Debug",
    perf=False,
    prof=False,
    coverage=False,
    sanitiser=SANITISER_NONE,
    sgx=FAASM_SGX_MODE_DISABLED,
    cpu=None,
    disable_spinlock=False,
):
    """
    Configures the CMake build
    """
    check_build_type(build)
    build_dir = get_build_dir(build, sgx)

    if clean and exists(build_dir):
        run("rm -rf {}/*".format(build_dir), shell=True, check=True)
        run(
            f"rm -rf {FAASM_BUILD_DIR}/bin".format(build_dir),
            shell=True,
            check=True,
        )

    if not exists(build_dir):
        makedirs(build_dir)

    if not exists(FAASM_INSTALL_DIR):
        makedirs(FAASM_INSTALL_DIR)

    conan_cache = f"{FAASM_CONAN_CACHE}/{build.lower()}"
    if not exists(conan_cache):
        print(f"ERROR: expected conan cache in {conan_cache}")
        print("ERROR: make sure to run 'inv dev.conan' first")
        raise RuntimeError(f"Expected conan cache in {conan_cache}")

    cmd = [
        "cmake",
        "-GNinja",
        f"-DCMAKE_TOOLCHAIN_FILE={conan_cache}/conan_toolchain.cmake",
        "-DCMAKE_BUILD_TYPE={}".format(build),
        "-DCMAKE_CXX_COMPILER=/usr/bin/clang++-{}".format(LLVM_MAJOR_VERSION),
        "-DCMAKE_C_COMPILER=/usr/bin/clang-{}".format(LLVM_MAJOR_VERSION),
        "-DCMAKE_INSTALL_PREFIX={}".format(FAASM_INSTALL_DIR),
        "-DFAASM_PERF_PROFILING=ON" if perf else "",
        "-DFAASM_CODE_COVERAGE=ON" if coverage else "",
        "-DFAASM_LLVM_MAJOR_VERSION={}".format(LLVM_MAJOR_VERSION),
        "-DFAASM_SELF_TRACING=ON" if prof else "",
        "-DFAABRIC_SELF_TRACING=ON" if prof else "",
        "-DFAASM_USE_SANITISER={}".format(sanitiser),
        "-DFAABRIC_USE_SANITISER={}".format(sanitiser),
        "-DFAABRIC_USE_SPINLOCK={}".format(
            "OFF" if disable_spinlock else "ON"
        ),
        "-DFAASM_SGX_MODE={}".format(sgx),
        "-DFAASM_TARGET_CPU={}".format(cpu) if cpu else "",
        get_dict_as_cmake_vars(FAASM_RUNTIME_ENV_DICT),
        PROJ_ROOT,
    ]

    cmd_str = " ".join(cmd)
    print(cmd_str)

    run(cmd_str, shell=True, check=True, cwd=build_dir)

    soft_link_bin_dir(build_dir)


@task
def tools(
    ctx,
    clean=False,
    build="Debug",
    parallel=0,
    sanitiser=SANITISER_NONE,
    sgx=FAASM_SGX_MODE_DISABLED,
    coverage=False,
    disable_spinlock=False,
):
    """
    Builds all the targets commonly used for development
    """
    if sgx != FAASM_SGX_MODE_DISABLED and sanitiser != SANITISER_NONE:
        raise RuntimeError("SGX and sanitised builds are incompatible!")

    build_dir = get_build_dir(build, sgx)

    conan(ctx, clean=clean, build=build)

    cmake(
        ctx,
        clean=clean,
        build=build,
        sanitiser=sanitiser,
        sgx=sgx,
        coverage=coverage,
        disable_spinlock=disable_spinlock,
    )

    targets = " ".join(DEV_TARGETS)

    cmake_cmd = "cmake --build . --target {}".format(targets)
    if parallel > 0:
        cmake_cmd += " --parallel {}".format(parallel)
    print(cmake_cmd)
    run(
        cmake_cmd,
        cwd=build_dir,
        shell=True,
        check=True,
    )

    soft_link_bin_dir(build_dir)


@task
def cc(ctx, target, clean=False, parallel=0):
    """
    Compiles the given CMake target
    """
    build_dir = get_current_target_build_dir()

    if clean:
        cmake(ctx, clean=True)

    if target == "all":
        target = ""
    else:
        target = "--target {}".format(target)

    cmake_cmd = "cmake --build . {}".format(target)
    if parallel > 0:
        cmake_cmd += " --parallel {}".format(parallel)

    run(
        cmake_cmd,
        cwd=build_dir,
        shell=True,
        check=True,
    )


@task
def coverage_report(ctx, file_in, file_out):
    """
    Generate code coverage report
    """
    tmp_file = "tmp_gha.profdata"

    # First, merge in the raw profiling data
    llvm_cmd = [
        "llvm-profdata-{}".format(LLVM_MAJOR_VERSION),
        "merge -sparse {}".format(file_in),
        "-o {}".format(tmp_file),
    ]
    llvm_cmd = " ".join(llvm_cmd)
    run(llvm_cmd, shell=True, check=True, cwd=PROJ_ROOT)

    # Second, generate the coverage report
    llvm_cmd = [
        "llvm-cov-{} show".format(LLVM_MAJOR_VERSION),
        "--ignore-filename-regex=/usr/local/code/faasm/tests/*",
        join(FAASM_BUILD_DIR, "debug", "bin", "tests"),
        "-instr-profile={}".format(tmp_file),
        "> {}".format(file_out),
    ]
    llvm_cmd = " ".join(llvm_cmd)
    run(llvm_cmd, shell=True, check=True, cwd=PROJ_ROOT)
