import subprocess
from subprocess import check_output

from invoke import task


@task
def ibm_deploy_worker(ctx, update=False):
    cmd = [
        "ibmcloud",
        "fn",
        "action",
        "update" if update else "create",
        "faasm-worker",
        "--docker",
        "faasm/ibm-worker:latest"
    ]

    cmd_string = " ".join(cmd)
    print(cmd_string)
    check_output(cmd_string, shell=True)

    print("Done")


@task
def ibm_invoke_worker(ctx):
    cmd = [
        "ibmcloud",
        "fn",
        "action",
        "invoke",
        "--blocking",
        "faasm-worker"
    ]

    cmd_string = " ".join(cmd)
    print(cmd_string)
    check_output(cmd_string, shell=True)
    print("Done")


@task
def ibm_list(ctx):
    cmd = [
        "ibmcloud",
        "fn",
        "action",
        "list",
    ]

    cmd_string = " ".join(cmd)
    print(cmd_string)
    check_output(cmd_string, shell=True)
    print("Done")
