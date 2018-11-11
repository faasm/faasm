from os import makedirs
from os.path import dirname, realpath, join, exists, expanduser
from subprocess import call

HOME_DIR = expanduser("~")
PROJ_ROOT = dirname(dirname(realpath(__file__)))

DOWNLOAD_DIR = join(HOME_DIR, "faasm")

ANSIBLE_DIR = join(PROJ_ROOT, "ansible")

WASM_DIR = join(PROJ_ROOT, "wasm")


def get_wasm_func_path(user, func_name):
    func_dir = join(WASM_DIR, user, func_name)

    if not exists(func_dir):
        makedirs(func_dir, exist_ok=True, mode=0o775)

    return join(func_dir, "function.wasm")


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


def sudo_script(script_name):
    script_path = join("bin", script_name)

    cmd = [
        "sudo",
        "./{}".format(script_path),
    ]
    cmd = " ".join(cmd)

    print(cmd)
    call(cmd, shell=True, cwd=PROJ_ROOT)
