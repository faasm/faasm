from os import remove
from os.path import exists, join
from subprocess import call

from compile.env import PROJ_ROOT


def wasm_to_wast(args):
    """
    Converts a function's wasm file to wast
    """

    func_dir = join(PROJ_ROOT, "wasm", args.user, args.function)
    wasm_path = join(func_dir, "function.wasm")
    wast_path = join(func_dir, "function.wast")

    if not exists(wasm_path):
        print("Could not find wasm file at {}".format(wasm_path))
        exit(1)

    if exists(wast_path):
        remove(wast_path)

    disassemble_bin = join(PROJ_ROOT, "cmake-build-debug", "WAVM", "bin", "wavm-disas")

    cmd = [
        disassemble_bin,
        wasm_path,
        wast_path
    ]

    print("Calling: {}".format(" ".join(cmd)))
    call(cmd, cwd=func_dir)
