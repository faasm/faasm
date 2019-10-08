from os import remove
from os.path import exists, join
from subprocess import call

from invoke import task

from tasks.util.config import get_faasm_config
from tasks.util.env import PROJ_ROOT


@task
def redis_clear_queue(ctx, docker_compose=False, knative=False, ibm=False):
    if docker_compose:
        cmd = [
            "docker-compose", "exec", "redis-queue", "redis-cli", "flushall"
        ]
    elif ibm:
        faasm_conf = get_faasm_config()
        cmd = [
            "redis-cli",
            "-h {}".format(faasm_conf["IBM"]["redis_host_public"]),
            "flushall"
        ]
    elif knative:
        cmd = [
            "kubectl",
            "exec",
            "-n faasm",
            "redis-queue",
            "--",
            "redis-cli flushall",
        ]
    else:
        cmd = ["redis-cli", "flushall"]

    cmd_string = " ".join(cmd)
    ret_code = call(" ".join(cmd), shell=True, cwd=PROJ_ROOT)

    if ret_code != 0:
        print("Command failed: {}".format(cmd_string))
