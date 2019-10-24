from subprocess import call

from tasks.util.env import ANSIBLE_ROOT


def _ansible_playbook(playbook_name):
    cmd = [
        "ansible-playbook",
        "-i inventory/billing.yml",
        "{}.yml".format(playbook_name)
    ]

    cmd_str = " ".join(cmd)
    print(cmd_str)
    res = call(cmd_str, cwd=ANSIBLE_ROOT)
    if res != 0:
        print("Ansible command failed: {}".format(cmd_str))
        raise RuntimeError("Ansible command failed: {}".format(cmd_str))


def start_billing():
    _ansible_playbook("billing_start")


def pull_billing():
    _ansible_playbook("billing_pull")
