import shutil
from os.path import join, exists
from subprocess import call

from faasmcli.util.env import POSSIBLE_BUILD_BINS


def find_command(bin_name, dirs):
    # First check on the path
    found_cmd = shutil.which(bin_name)
    if found_cmd:
        return found_cmd

    found_cmds = list()
    if dirs:
        # If not found on the path, check in provided directories
        possible_files = [join(d, bin_name) for d in dirs]
        found_cmds = [b for b in possible_files if exists(b)]

    if len(found_cmds) == 0:
        raise RuntimeError("Could not find command for {}".format(bin_name))

    found_cmd = found_cmds[0]
    if len(found_cmds) > 1:
        print("WARNING: found multiple candidates for {}, taking {}".format(bin_name, found_cmd))

    return found_cmd


def run_command(bin_name, args=None):
    cmd_path = find_command(bin_name, POSSIBLE_BUILD_BINS)

    cmd = [cmd_path]

    if args:
        cmd.extend(args)

    cmd = " ".join(cmd)
    print(cmd)
    res = call(cmd, shell=True)

    if res != 0:
        print("Failed running {}".format(bin_name))
        exit(1)
