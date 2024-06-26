from faasmtools.build import FAASM_RUNTIME_ENV_DICT, get_dict_as_cmake_vars
from invoke import task
from os import makedirs
from os.path import exists, join
from subprocess import run
from tasks.util.env import (
    FAASM_BUILD_DIR,
    FAASM_INSTALL_DIR,
    FAASM_SGX_MODE_DISABLED,
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
    if clean and exists(FAASM_BUILD_DIR):
        run("rm -rf {}/*".format(FAASM_BUILD_DIR), shell=True, check=True)

    if not exists(FAASM_BUILD_DIR):
        makedirs(FAASM_BUILD_DIR)

    if not exists(FAASM_INSTALL_DIR):
        makedirs(FAASM_INSTALL_DIR)

    cmd = [
        "cmake",
        "-GNinja",
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

    run(cmd_str, shell=True, check=True, cwd=FAASM_BUILD_DIR)


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
        cwd=FAASM_BUILD_DIR,
        shell=True,
        check=True,
    )


@task
def cc(ctx, target, clean=False, parallel=0):
    """
    Compiles the given CMake target
    """
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
        cwd=FAASM_BUILD_DIR,
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
        join(FAASM_BUILD_DIR, "bin", "tests"),
        "-instr-profile={}".format(tmp_file),
        "> {}".format(file_out),
    ]
    llvm_cmd = " ".join(llvm_cmd)
    run(llvm_cmd, shell=True, check=True, cwd=PROJ_ROOT)
