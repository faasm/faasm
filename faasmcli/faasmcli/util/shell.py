import shutil
from os.path import join, exists
from subprocess import call

from faasmcli.util.env import FAASM_BUILD_DIR


def find_command(bin_name):
    # First check on the path
    found_cmd = shutil.which(bin_name)
    if found_cmd:
        print("Found {} in PATH".format(bin_name))
        return found_cmd

    # If not found on the path, check in build dir
    bin_path = join(FAASM_BUILD_DIR, "bin", bin_name)
    if not exists(bin_path):
        raise RuntimeError(
            "Could not find binary {} at {}".format(bin_name, bin_path)
        )

    print("Taking {} as {}".format(bin_name, found_cmd))
    return found_cmd


def run_command(bin_name, args=None):
    cmd_path = find_command(bin_name)

    cmd = [cmd_path]

    if args:
        cmd.extend(args)

    cmd = " ".join(cmd)
    print(cmd)
    res = call(cmd, shell=True)

    if res != 0:
        print("Failed running {}".format(bin_name))
        exit(1)
