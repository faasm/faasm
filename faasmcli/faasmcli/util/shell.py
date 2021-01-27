from copy import copy
from os.path import join, exists
from os import environ
import shutil
from subprocess import run

from faasmcli.util.env import FAASM_BUILD_DIR


def find_command(bin_name):
    # First check on the path
    bin_path = shutil.which(bin_name)
    if bin_path:
        print("Found {} in PATH".format(bin_name))
        return bin_path

    # If not found on the path, check in build dir
    bin_path = join(FAASM_BUILD_DIR, "bin", bin_name)
    if not exists(bin_path):
        raise RuntimeError(
            "Could not find binary {} at {}".format(bin_name, bin_path)
        )

    print("Taking {} as {}".format(bin_name, bin_path))
    return bin_path


def run_command(bin_name, args=None, extra_env=None):
    cmd_path = find_command(bin_name)

    cmd = [cmd_path]

    if args:
        cmd.extend(args)

    env = copy(environ)
    if extra_env:
        env.update(extra_env)

    cmd = " ".join(cmd)
    print(cmd)
    run(cmd, shell=True, check=True, env=env)
