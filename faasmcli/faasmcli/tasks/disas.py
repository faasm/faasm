from os.path import join, exists

from invoke import task
from subprocess import run

from faasmcli.util.env import WASM_DIR
from faasmcli.util.shell import run_command


def _do_disas(user, func):
    args = [user, func]
    run_command("func_sym", args)

    # Print out the results
    syms_path = join(WASM_DIR, user, func, "function.symbols")
    if not exists(syms_path):
        print("Did not find symbols at {}".format(syms_path))
        exit(1)

    return syms_path


@task(default=True)
def symbols(ctx, user, func):
    """
    Print out the symbols for this function
    """
    syms_path = _do_disas(user, func)

    with open(syms_path) as fh:
        print(fh.read())

    print("\nSymbols written to {}".format(syms_path))


@task
def replace(ctx, user, func, file_in):
    """
    Replaces the original symbols with those from the disassembly in the given
    file.
    """
    syms_path = _do_disas(user, func)

    print("Replacing symbols for {}/{} in {}".format(user, func, file_in))

    # Iterate through symbols and do a brute force replace on each
    # Probably a more efficient way to do this.
    with open(syms_path, "r") as fh:
        for line in fh:
            line = line.strip()
            if not line:
                continue

            symbol, disas = line.split(":")

            sed_cmd = [
                "sed",
                "-i",
                "'s/{}/{}/g'".format(symbol, disas),
                file_in,
            ]
            sed_cmd = " ".join(sed_cmd)
    #        print(sed_cmd)
            run(sed_cmd, shell=True, check=True)
