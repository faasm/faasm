from faasmcli.util.shell import run_command
from invoke import task
from os import getenv


def do_run_command(cmd_name, user, function, data, cmdline):
    args = [user, function]
    if data:
        args.append("--input-data '{}'".format(data))
    if cmdline:
        args.append("--cmdline '{}'".format(cmdline))

    wasm_vm = getenv("WASM_VM", default="wavm")
    extra_env = {"WASM_VM": wasm_vm}

    run_command(cmd_name, args, extra_env=extra_env)


@task(default=True)
def run(ctx, user, function, data=None, cmdline=None):
    """
    Execute a specific function using a single Faaslet
    """
    do_run_command("func_runner", user, function, data, cmdline)


@task()
def pool(ctx, user, function, data=None, cmdline=None):
    """
    Execute a specific function using a pool of Faaslets
    """
    do_run_command("local_pool_runner", user, function, data, cmdline)
