from os import makedirs
from os.path import exists
from subprocess import run

from invoke import task

from faasmcli.util.env import PROJ_ROOT, FAASM_BUILD_DIR, FAASM_INSTALL_DIR

DEV_TARGETS = [
    "codegen_func",
    "codegen_shared_obj",
    "func_runner",
    "func_sym",
    "pool_runner",
    "upload",
    "tests",
]


@task
def cmake(
    ctx,
    clean=False,
    build="Debug",
    perf=False,
    prof=False,
    sanitiser="None",
    nosgx=False,
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
        "-DCMAKE_CXX_COMPILER=/usr/bin/clang++-13",
        "-DCMAKE_C_COMPILER=/usr/bin/clang-13",
        "-DCMAKE_INSTALL_PREFIX={}".format(FAASM_INSTALL_DIR),
        "-DFAASM_PERF_PROFILING=ON" if perf else "",
        "-DFAASM_SELF_TRACING=ON" if prof else "",
        "-DFAABRIC_SELF_TRACING=ON" if prof else "",
        "-DFAASM_USE_SANITISER={}".format(sanitiser),
        "-DFAABRIC_USE_SANITISER={}".format(sanitiser),
        "-DFAASM_SGX={}".format("OFF" if nosgx else "ON"),
        PROJ_ROOT,
    ]

    cmd_str = " ".join(cmd)
    print(cmd_str)
    run(cmd_str, shell=True, check=True, cwd=FAASM_BUILD_DIR)


@task
def tools(ctx, clean=False, build="Debug", parallel=0, sanitiser="None"):
    """
    Builds all the targets commonly used for development
    """
    nosgx = True if sanitiser is not None else False
    cmake(ctx, clean=clean, build=build, sanitiser=sanitiser, nosgx=nosgx)

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
