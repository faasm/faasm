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

    # Iterate through symbols to build up *ordered* list of substitutions.
    # We have to start with largest numbers first, so that functionDef9 doesn't
    # replace the start of functionDef996 for example
    substitutions = list()
    with open(syms_path, "r") as fh:
        for line in fh:
            line = line.strip()
            if not line:
                continue

            symbol, disas = line.split(" = ")

            # Work out the number of the function def/ import
            symbol = symbol.strip()
            if symbol.startswith("functionImport"):
                number = int(symbol.replace("functionImport", ""))
            elif symbol.startswith("functionDef"):
                number = int(symbol.replace("functionDef", ""))
            else:
                raise RuntimeError("Unrecognised symbol: {}".format(symbol))

            # Remove symbols that might mess up an output file
            banned_chars = ["(", ")", ",", ":", "<", ">"]
            for c in banned_chars:
                disas = disas.replace(c, "_")

            if prefix:
                disas = "{}_{}".format(prefix, disas)

            substitutions.append((number, symbol, disas))

        # Now we do the sorting which we *must* do to avoid erroneous partial
        # substitutions
        substitutions.sort(key=lambda x: x[0], reverse=True)

        # Now we do the actual substitutions
        for subs in substitutions:
            sed_cmd = [
                "sed",
                "-i",
                "'s/{}/{}/g'".format(subs[1], subs[2]),
                file_path,
            ]
            sed_cmd = " ".join(sed_cmd)
            print(sed_cmd)
            run(sed_cmd, shell=True, check=True)
