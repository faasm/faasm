from invoke import task
from os import getenv

from faasmcli.util.shell import run_command


@task(default=True)
def run(ctx, user, function, data=None, cmdline=None):
    """
    Execute a specific function
    """
    args = [user, function]
    if data:
        args.append("--input-data '{}'".format(data))
    if cmdline:
        args.append("--cmdline '{}'".format(cmdline))

    wasm_vm = getenv("WASM_VM", default="wavm")
    extra_env = {"WASM_VM": wasm_vm}

    run_command("func_runner", args, extra_env=extra_env)
