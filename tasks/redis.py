from subprocess import call

from invoke import task

from tasks.util.config import get_faasm_config
from tasks.util.env import PROJ_ROOT


def _do_redis_command(sub_cmd, local, docker, knative, ibm):
    if local:
        cmd = [
            "redis-cli", sub_cmd
        ]
    elif docker:
        cmd = [
            "docker-compose", "exec", "redis-queue", "redis-cli", sub_cmd
        ]
    elif ibm:
        faasm_conf = get_faasm_config()
        cmd = [
            "redis-cli",
            "-h {}".format(faasm_conf["IBM"]["redis_host_public"]),
            sub_cmd
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
def clear_queue(ctx, local=False, docker=False, knative=True, ibm=False):
    _do_redis_command("flushall", local, docker, knative, ibm)


@task
def all_workers(ctx, local=False, docker=False, knative=True, ibm=False):
    _do_redis_command("smembers available_workers", local, docker, knative, ibm)


@task
def func_workers(ctx, user, func, local=False, docker=False, knative=True, ibm=False):
    worker_set_name = "w_{}/{}".format(user, func)
    _do_redis_command("smembers {}".format(worker_set_name), local, docker, knative, ibm)
