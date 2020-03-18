from os.path import join, exists

from invoke import task

from tasks.util.env import get_wasm_func_path, WASM_DIR
from tasks.util.shell import run_command


@task(default=True)
def symbols(ctx, user, func):
    """
    Print out the symbols for this function
    """
    args = [user, func]
    run_command("func_sym", args)

    # Print out the results
    syms_path = join(WASM_DIR, user, func, "function.symbols")
    if not exists(syms_path):
        print("Did not find symbols at {}".format(syms_path))
        exit(1)

    with open(syms_path) as fh:
        print(fh.read())
