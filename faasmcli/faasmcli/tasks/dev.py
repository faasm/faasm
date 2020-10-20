from os import makedirs
from os.path import exists
from shutil import rmtree
from subprocess import run

from invoke import task

from faasmcli.util.env import PROJ_ROOT
from faasmcli.util.shell import get_local_build_dir


@task
def cmake(ctx, clean=False):
    build_dir = get_local_build_dir()
    if clean and exists(build_dir):
        rmtree(build_dir)

    if not exists(build_dir):
        makedirs(build_dir)

    cmd = [
        "cmake",
        "-GNinja",
        "-DCMAKE_BUILD_TYPE=Debug",
        "-DCMAKE_CXX_COMPILER=/usr/bin/clang++-10",
        "-DCMAKE_C_COMPILER=/usr/bin/clang-10",
        PROJ_ROOT,
    ]

    run(" ".join(cmd), shell=True, cwd=build_dir)


@task
def cc(ctx, target, clean=False):
    build_dir = get_local_build_dir()

    if clean:
        if exists(build_dir):
            rmtree(build_dir)

        cmake(ctx, clean=True)

    if target == "all":
        target = ""

    run(
        "ninja {}".format(target),
        cwd=build_dir,
        shell=True,
    )
