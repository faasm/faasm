from invoke import task

from faasmcli.util.shell import run_command


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
    args = [user, function]
    if repeats:
        args.append(str(repeats))

    run_command("simple_runner", args)
