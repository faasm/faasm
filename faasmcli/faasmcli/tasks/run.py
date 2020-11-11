from copy import copy
from invoke import task
from os import environ

from faasmcli.util.shell import run_command


@task
def user(ctx, user, repeats=1, wamr=False):
    """
    Execute all the functions for the given user
    """
    run(ctx, user, "all", repeats=repeats, wamr=wamr)


@task(default=True)
def run(ctx, user, function, repeats=1, wamr=False):
    """
    Execute a specific function
    """
    args = [user, function]
    if repeats:
        args.append(str(repeats))

    wasm_vm = "wamr" if wamr else "wavm"
    env = copy(environ)
    env.update({"WASM_VM": wasm_vm})

    run_command("simple_runner", args)
