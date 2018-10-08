from os.path import dirname, realpath, join
from subprocess import call

PROJ_ROOT = dirname(dirname(realpath(__file__)))

ANSIBLE_DIR = join(PROJ_ROOT, "ansible")


def playbook_command(playbook_name, sudo=True, inventory=None):
    cmd = [
        "ansible-playbook",
        playbook_name,
        "--ask-become-pass" if sudo else "",
        "-i inventory/{}".format(inventory) if inventory else "",
    ]

    cmd = " ".join(cmd)

    print(cmd)
    call(cmd, shell=True, cwd=ANSIBLE_DIR)
