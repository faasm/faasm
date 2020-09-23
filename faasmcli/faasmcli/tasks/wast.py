from os import remove
from os.path import exists, join
from subprocess import run

from invoke import task

from faasmcli.util.env import PROJ_ROOT, THIRD_PARTY_DIR
from faasmcli.util.files import clean_dir

WABT_DIR = join(THIRD_PARTY_DIR, "wabt")
WABT_BUILD_DIR = join(WABT_DIR, "build")
WASM2WAT_BIN = join(WABT_BUILD_DIR, "wasm2wat")
DECOMPILE_BIN = join(WABT_BUILD_DIR, "wasm-decompile")


@task
def build_wabt(ctx, clean=False):
    """
    Builds wabt
    """

    clean_dir(WABT_BUILD_DIR, clean)

    cmake_cmd = ["cmake", "-DBUILD_TESTS=OFF", "-GNinja", ".."]

    run(" ".join(cmake_cmd), shell=True, check=True, cwd=WABT_BUILD_DIR)

    targets = ["wasm2wat", "wasm-decompile"]
    for target in targets:
        run(
            "ninja {}".format(target),
            shell=True,
            check=True,
            cwd=WABT_BUILD_DIR,
        )


@task
def file(ctx, file_path):
    """
    Generate .wast file from a .wasm file
    """
    _do_wast(file_path, "/tmp/out.wast")


@task(default=True)
def wast(ctx, user, func_name):
    """
    Generate .wast file for a given function
    """

    func_dir = join(PROJ_ROOT, "wasm", user, func_name)
    wasm_path = join(func_dir, "function.wasm")
    wast_path = join(func_dir, "function.wast")
    _do_wast(wasm_path, wast_path)


@task
def decompile(ctx, user, func_name):
    """
    Decompiles the given function
    """
    func_dir = join(PROJ_ROOT, "wasm", user, func_name)
    wasm_path = join(func_dir, "function.wasm")
    decomp_file = join(func_dir, "function.dcmp")

    cmd = [
        DECOMPILE_BIN,
        wasm_path,
        "-o {}".format(decomp_file),
        "--enable-all",
    ]

    run(" ".join(cmd), shell=True, check=True)


def _do_wast(wasm_path, wast_path, cwd=None):
    if not exists(wasm_path):
        print("Could not find wasm file at {}".format(wasm_path))
        exit(1)

    if exists(wast_path):
        remove(wast_path)

    cmd = [
        WASM2WAT_BIN,
        "--enable-all",
        wasm_path,
        "-o {}".format(wast_path),
    ]

    extra_kwargs = dict()
    if cwd:
        extra_kwargs["cwd"] = cwd

    cmd = " ".join(cmd)
    print(cmd)
    run(cmd, shell=True, check=True, **extra_kwargs)

    # call("head -40 {}".format(wast_path), shell=True)
    print("vim {}".format(wast_path))
