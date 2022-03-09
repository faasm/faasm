from invoke import task
from faasmcli.util.shell import find_command
from subprocess import run


@task
def check(ctx):
    """
    Detect if SGX is supported
    """
    binary = find_command("detect_sgx")
    ret_code = run(binary, shell=True)
