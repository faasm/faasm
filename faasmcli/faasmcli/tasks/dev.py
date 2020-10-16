import os
from os import makedirs
from os.path import join, exists
from shutil import rmtree
from subprocess import run

from invoke import task

from faasmcli.util.env import PROJ_ROOT

# Build dir depends on whether we're in the Faasm Docker env
if os.environ.get("FAASM_DOCKER"):
    _BUILD_DIR = "/faasm/build"
else:
    _BUILD_DIR = join(PROJ_ROOT, "build", "cmake")

_BIN_DIR = join(_BUILD_DIR, "bin")


@task
def cmake(ctx, clean=False):
    if clean and exists(_BUILD_DIR):
        rmtree(_BUILD_DIR)

    if not exists(_BUILD_DIR):
        makedirs(_BUILD_DIR)
    
    cmd = [
        "cmake",
        "-GNinja",
        "-DCMAKE_BUILD_TYPE=Debug",
        "-DCMAKE_CXX_COMPILER=/usr/bin/clang++-10",
        "-DCMAKE_C_COMPILER=/usr/bin/clang-10",
        PROJ_ROOT
    ]

    run(" ".join(cmd), shell=True, cwd=_BUILD_DIR)


@task
def cc(ctx, target, clean=False):
    if clean:
        if exists(_BUILD_DIR):
            rmtree(_BUILD_DIR)

        cmake(ctx, clean=True)

    if target == "all":
        target = ""

    run(
        "ninja {}".format(target),
        cwd=_BUILD_DIR,
        shell=True,
    )


@task
def r(ctx, target):
    run(
        "./{}".format(target),
        cwd=_BIN_DIR,
        shell=True,
    )
