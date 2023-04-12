from subprocess import call

from invoke import task
from os import environ
from faasmcli.util.env import PROJ_ROOT


def _do_redis_command(sub_cmd, local, docker, k8s):
    redis_host = environ.get("REDIS_QUEUE_HOST", "redis")
    if local:
        cmd = ["redis-cli", sub_cmd]
    elif docker:
        cmd = ["redis-cli", "-h", redis_host, sub_cmd]
    elif k8s:
        cmd = [
            "kubectl",
            "exec",
            "-n faasm",
            "redis-queue",
            "--",
            "redis-cli",
            sub_cmd,
        ]
    else:
        cmd = ["redis-cli", sub_cmd]

    cmd_string = " ".join(cmd)
    print(cmd_string)
    ret_code = call(" ".join(cmd), shell=True, cwd=PROJ_ROOT)

    if ret_code != 0:
        print("Command failed: {}".format(cmd_string))


@task
def clear_queue(ctx, local=False, docker=False, k8s=True):
    """
    Clear the message queue in Redis
    """
    _do_redis_command("flushall", local, docker, k8s)


@task
def func_workers(ctx, user, func, local=False, docker=False, k8s=True):
    """
    List all warm Faasm instances
    """
    worker_set_name = "w_{}/{}".format(user, func)
    _do_redis_command(
        "smembers {}".format(worker_set_name), local, docker, k8s
    )
