from subprocess import call

from invoke import task

from tasks.util.env import POSSIBLE_BUILD_BINS
from tasks.util.shell import find_command


@task
def run_user(ctx, user, repeats=1):
    run(ctx, user, "all", repeats=repeats)


@task
def run(ctx, user, function, repeats=1):
    runner = find_command("simple_runner", POSSIBLE_BUILD_BINS)

    cmd = [runner, user]

    if function:
        cmd.append(function)

    if repeats:
        cmd.append(str(repeats))

    cmd = " ".join(cmd)
    print(cmd)
    res = call(cmd, shell=True)

    if res != 0:
        print("Failed running command")
        exit(1)
