from os import makedirs
from os.path import exists
from shutil import rmtree
from subprocess import run

from invoke import task

from faasmcli.util.env import PROJ_ROOT, FAASM_BUILD_DIR


@task
def cmake(ctx, clean=False):
    """
    Configures the CMake build
    """
    if clean and exists(FAASM_BUILD_DIR):
        run("rm -rf {}/*".format(FAASM_BUILD_DIR), shell=True, check=True)

    if not exists(FAASM_BUILD_DIR):
        makedirs(FAASM_BUILD_DIR)

    cmd = [
        "cmake",
        "-GNinja",
        "-DCMAKE_BUILD_TYPE=Debug",
        "-DCMAKE_CXX_COMPILER=/usr/bin/clang++-10",
        "-DCMAKE_C_COMPILER=/usr/bin/clang-10",
        PROJ_ROOT,
    ]

    run(" ".join(cmd), shell=True, cwd=FAASM_BUILD_DIR)


@task
def cc(ctx, target, clean=False):
    """
    Compiles the given CMake target
    """
    if clean:
        cmake(ctx, clean=True)

    if target == "all":
        target = ""

    run(
        "ninja {}".format(target),
        cwd=FAASM_BUILD_DIR,
        shell=True,
    )
