import shutil
from os.path import join, exists
from subprocess import call

from faasmcli.util.env import PROJ_ROOT, HOME_DIR


POSSIBLE_BUILD_DIRS = [
    "/faasm/build/",  # Containers
    join(PROJ_ROOT, "build"),  # Local builds
    join(PROJ_ROOT, "build", "cmake"),  # Local builds
    join(HOME_DIR, "faasm", "bench"),  # Benchmark
]


def get_local_build_dir():
    existing = [d for d in POSSIBLE_BUILD_DIRS if exists(d)]
    if len(existing) == 0:
        raise RuntimeError(
            "Could not find build dir (checking {})".format(
                POSSIBLE_BUILD_DIRS
            )
        )

    if len(existing) > 1:
        raise RuntimeError(
            "Found multiple potential build dirs: {}".format(existing)
        )

    return existing[0]


def find_command(bin_name):
    # First check on the path
    found_cmd = shutil.which(bin_name)
    if found_cmd:
        print("Found {} in PATH".format(bin_name))
        return found_cmd

    # If not found on the path, check in build dir
    bin_path = join(get_local_build_dir(), "bin", bin_name)
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
