from os import remove
from os.path import exists, join
from subprocess import call

from invoke import task

from tasks.util.env import PROJ_ROOT, POSSIBLE_BUILD_BINS
from tasks.util.shell import find_command


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


def _do_wast(wasm_path, wast_path, cwd=None):
    if not exists(wasm_path):
        print("Could not find wasm file at {}".format(wasm_path))
        exit(1)

    if exists(wast_path):
        remove(wast_path)

    wavm_bin = find_command("wavm", POSSIBLE_BUILD_BINS)

    cmd = [
        wavm_bin,
        "disassemble",
        wasm_path,
        wast_path,
        "--enable simd",
        "--enable atomics",
    ]

    cmd = " ".join(cmd)
    kwargs = {
        "shell": True,
    }
    if cwd:
        kwargs["cwd"] = cwd

    call(cmd, **kwargs)

    # call("head -40 {}".format(wast_path), shell=True)
    print("vim {}".format(wast_path))
