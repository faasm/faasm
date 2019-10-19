from os import remove
from os.path import exists, join
from subprocess import call

from invoke import task

from tasks.util.config import get_faasm_config
from tasks.util.env import PROJ_ROOT


def _do_redis_command(sub_cmd, docker, knative, ibm):
    if docker:
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
def redis_clear_queue(ctx, docker=False, knative=True, ibm=False):
    _do_redis_command("flushall", docker, knative, ibm)


@task
def redis_all_workers(ctx, docker=False, knative=True, ibm=False):
    _do_redis_command("smembers available_workers", docker, knative, ibm)


@task
def redis_func_workers(ctx, user, func, docker=False, knative=True, ibm=False):
    worker_set_name = "w_{}/{}".format(user, func)
    _do_redis_command("smembers {}".format(worker_set_name), docker, knative, ibm)
