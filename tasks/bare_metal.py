from os.path import join
from subprocess import call

from invoke import task

from tasks.util.env import ANSIBLE_ROOT

DEFAULT_INVENTORY = join(ANSIBLE_ROOT, "inventory", "mpi.yml")


def _ansible_command(host_group, cmd, inventory=DEFAULT_INVENTORY):
    shell_cmd = [
        "ansible",
        "-i", inventory,
        host_group,
        "-a", "\"{}\"".format(cmd)
    ]

    shell_cmd = " ".join(shell_cmd)
    print(shell_cmd)
    res = call(shell_cmd, shell=True, cwd=ANSIBLE_ROOT)
    if res != 0:
        print("Failed Ansible command")
        exit(1)


@task
def bm_restart_workers(ctx):
    _ansible_command("worker", "sudo supervisorctl restart faasm_worker")


@task
def bm_restart(ctx):
    _ansible_command("redis", "redis-cli flushall")
    _ansible_command("worker", "sudo supervisorctl restart faasm_worker")
    _ansible_command("upload", "sudo supervisorctl restart faasm_upload")
