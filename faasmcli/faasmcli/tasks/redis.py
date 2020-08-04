from subprocess import call

from invoke import task

from faasmcli.util.config import get_faasm_config
from faasmcli.util.env import PROJ_ROOT


def _do_redis_command(sub_cmd, local, docker, knative):
    if local:
        cmd = [
            "redis-cli", sub_cmd
        ]
    elif docker:
        cmd = [
            "docker-compose", "exec", "redis-queue", "redis-cli", sub_cmd
        ]
    elif knative:
        cmd = [
            "kubectl",
            "exec",
            "-n faasm",
            "redis-queue",
            "--",
            "redis-cli", sub_cmd
        ]
    else:
        cmd = ["redis-cli", sub_cmd]

    cmd_string = " ".join(cmd)
    print(cmd_string)
    ret_code = call(" ".join(cmd), shell=True, cwd=PROJ_ROOT)

    if ret_code != 0:
        print("Command failed: {}".format(cmd_string))


@task
def clear_queue(ctx, local=False, docker=False, knative=True):
    """
    Clear the message queue in Redis
    """
    _do_redis_command("flushall", local, docker, knative)


@task
def all_workers(ctx, local=False, docker=False, knative=True):
    """
    List all available Faasm instances
    """
    _do_redis_command("smembers available_faaslets", local, docker, knative)


@task
def func_workers(ctx, user, func, local=False, docker=False, knative=True):
    """
    List all warm Faasm instances
    """
    worker_set_name = "w_{}/{}".format(user, func)
    _do_redis_command("smembers {}".format(worker_set_name), local, docker, knative)
