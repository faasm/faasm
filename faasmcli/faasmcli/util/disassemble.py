from os.path import join, exists

from subprocess import run

from faasmcli.util.env import WASM_DIR
from faasmcli.util.shell import run_command


def disassemble_function(user, func):
    args = [user, func]
    run_command("func_sym", args)

    # Print out the results
    syms_path = join(WASM_DIR, user, func, "function.symbols")
    if not exists(syms_path):
        print("Did not find symbols at {}".format(syms_path))
        exit(1)

    return syms_path


def replace_symbols_in_file(user, func, file_path, prefix=None):
    syms_path = disassemble_function(user, func)

    # Iterate through symbols and do a brute force replace on each
    # Probably a more efficient way to do this.
    with open(syms_path, "r") as fh:
        for line in fh:
            line = line.strip()
            if not line:
                continue

            symbol, disas = line.split(" = ")

            replacement = "{}__{}".format(prefix, disas) if prefix else disas

            sed_cmd = [
                "sed",
                "-i",
                "'s/{}/{}/g'".format(symbol, replacement),
                file_path,
            ]
            sed_cmd = " ".join(sed_cmd)
            print(sed_cmd)
            run(sed_cmd, shell=True, check=True)
