import shutil
from os.path import join, exists


def find_command(bin_name, dirs):
    # First check on the path
    found_cmd = shutil.which(bin_name)
    if found_cmd:
        return found_cmd

    # If not found on the path, check in provided directories
    possible_files = [join(d, bin_name) for d in dirs]
    found_cmds = [b for b in possible_files if exists(b)]

    found_cmd = found_cmds[0]
    if len(found_cmds) > 1:
        print("WARNING: found multiple candidates for {}, taking {}".format(bin_name, found_cmd))

    return found_cmd
