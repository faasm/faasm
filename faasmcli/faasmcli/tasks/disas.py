from invoke import task

from faasmcli.util.disassemble import (
    disassemble_function,
    replace_symbols_in_file,
)


@task(default=True)
def symbols(ctx, user, func):
    """
    Print out the symbols for this function
    """
    syms_path = disassemble_function(user, func)

    with open(syms_path) as fh:
        print(fh.read())

    print("\nSymbols written to {}".format(syms_path))


@task
def replace(ctx, user, func, file_in):
    """
    Replaces the original symbols with those from the disassembly in the given
    file.
    """
    print("Replacing symbols for {}/{} in {}".format(user, func, file_in))
    replace_symbols_in_file(user, func, file_in)
