from invoke import task
from os import getenv

from faasmcli.util.shell import run_command


@task(default=True)
def run(ctx, user, function, data=None):
    """
    Execute a specific function
    """
    args = [user, function]
    if data:
        args.append(data)

    wasm_vm = getenv("WASM_VM", default="wavm")
    extra_env = {"WASM_VM": wasm_vm}

    run_command("func_runner", args, extra_env=extra_env)
