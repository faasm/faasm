from subprocess import call

from invoke import task

from tasks.util.env import POSSIBLE_BUILD_BINS
from tasks.util.shell import find_command


@task
def user(ctx, user, repeats=1):
    """
    Execute all the functions for the given user
    """
    run(ctx, user, "all", repeats=repeats)


@task(default=True)
def run(ctx, user, function, repeats=1):
    """
    Execute a specific function
    """
    runner = find_command("simple_runner", POSSIBLE_BUILD_BINS)

    cmd = [runner, user, function]

    if repeats:
        cmd.append(str(repeats))

    cmd = " ".join(cmd)
    print(cmd)
    res = call(cmd, shell=True)

    if res != 0:
        print("Failed running command")
        exit(1)
