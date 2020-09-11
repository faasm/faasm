from os.path import join
from subprocess import call

from invoke import task

from faasmcli.util.env import ANSIBLE_ROOT

DEFAULT_INVENTORY = join(ANSIBLE_ROOT, "inventory", "bare_metal.yml")


def _call_ansible_command(cmd):
    shell_cmd = " ".join(cmd)
    print(shell_cmd)
    res = call(shell_cmd, shell=True, cwd=ANSIBLE_ROOT)
    if res != 0:
        print("Ansible command failed")
        exit(1)


def _ansible_playbook_command(playbook, inventory=DEFAULT_INVENTORY, extra_vars=None):
    shell_cmd = [
        "ansible-playbook",
        "-i", inventory,
        playbook
    ]

    if extra_vars:
        for var_name, var_value in extra_vars.items():
            shell_cmd.append("--extra-vars {}={}".format(var_name, var_value))

    _call_ansible_command(shell_cmd)


def _ansible_command(host_group, cmd, inventory=DEFAULT_INVENTORY):
    shell_cmd = [
        "ansible",
        "-i", inventory,
        host_group,
        "-a", "\"{}\"".format(cmd)
    ]
    _call_ansible_command(shell_cmd)


@task
def setup(ctx):
    """
    Run the initial machine set-up
    """
    _ansible_playbook_command("bare_metal_base.yml")


@task
def deploy(ctx, quick=False, branch=None):
    """
    Run bare metal deploy
    """
    extra_vars = dict()
    if quick:
        extra_vars["quick_deploy"] = "on"

    if branch:
        extra_vars["repo_version"] = branch

    _ansible_playbook_command("faasm_bare.yml", extra_vars=extra_vars)


@task
def restart_workers(ctx):
    """
    Restart bare metal workers
    """
    # Clear out machine code
    _ansible_command("worker", "rm -rf /usr/local/faasm/object")

    # Restart
    _ansible_command("worker", "sudo supervisorctl restart faasm_worker")


@task
def restart(ctx):
    """
    Restart whole bare metal deployment
    """

    # Flush redis
    _ansible_command("redis", "redis-cli flushall")

    # Clear out any function machine code
    _ansible_command("worker", "rm -rf /usr/local/faasm/object")

    # Restart the application
    _ansible_command("upload", "sudo supervisorctl restart faasm_upload")
    _ansible_command("worker", "sudo supervisorctl restart faasm_worker")


@task
def stop(ctx):
    """
    Stops the Faasm bare metal application
    """
    _ansible_command("all", "sudo supervisorctl stop all")


@task
def uninstall(ctx):
    """
    Uninstalls the Faasm bare metal application
    """
    _ansible_command("all", "sudo supervisorctl stop all")
    _ansible_command("all", "sudo rm -rf /etc/supervisor/conf.d/*")
    _ansible_command("all", "sudo supervisorctl update all")
