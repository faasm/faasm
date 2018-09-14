# ----------------------------------
# Script to convert wasm to wast
# ----------------------------------

import argparse
from os import remove
from os.path import exists, dirname, realpath, join
from subprocess import call

PROJ_ROOT = dirname(dirname(realpath(__file__)))

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("user", help="Owner of the function")
    parser.add_argument("function", help="Function name")

    args = parser.parse_args()

    func_dir = join(PROJ_ROOT, "wasm", args.user, args.function)
    wasm_path = join(func_dir, "function.wasm")
    wast_path = join(func_dir, "function.wast")

    if not exists(wasm_path):
        print("Could not find wasm file at {}".format(wasm_path))
        exit(1)

    if exists(wast_path):
        remove(wast_path)

    disassemble_bin = join(PROJ_ROOT, "cmake-build-debug", "WAVM", "bin", "Disassemble")

    cmd = [
        disassemble_bin,
        wasm_path,
        wast_path
    ]

    print("Calling: {}".format(" ".join(cmd)))
    call(cmd, cwd=func_dir)
