from subprocess import call

from invoke import task

from tasks.util.env import FUNC_NATIVE_BUILD_DIR

@task
def native_run(ctx, target, ltrace=False, strace=False):
    cmd = "./{}".format(target)

    cmd = "ltrace -l libomp.so.5 {}".format(cmd) if ltrace else cmd
    cmd = "strace {}".format(cmd) if strace else cmd

    res = call(cmd, shell=True, cwd=FUNC_NATIVE_BUILD_DIR)
    if res != 0:
        print("Failed to compile")
