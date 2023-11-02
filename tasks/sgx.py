from invoke import task
from subprocess import run
from tasks.util.shell import find_command


@task
def check(ctx):
    """
    Detect if SGX is supported
    """
    binary = find_command("detect_sgx")
    run(binary, shell=True)
