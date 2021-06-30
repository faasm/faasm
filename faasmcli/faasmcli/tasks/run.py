from invoke import task
from sys import exit

from faasmcli.util.shell import run_command


@task(default=True)
def run(ctx, user, function, wamr=False, data=None, sgx=False):
    """
    Execute a specific function
    """
    args = [user, function]
    if data:
        args.append(data)
    if sgx:
        args.append("--sgx")

    wasm_vm = "wamr" if wamr else "wavm"
    extra_env = {"WASM_VM": wasm_vm}

    if (wasm_vm != "wamr") and sgx:
        print("Can't run SGX functions with WAVM. Add --wamr flag.")
        exit(1)

    run_command("func_runner", args, extra_env=extra_env)
