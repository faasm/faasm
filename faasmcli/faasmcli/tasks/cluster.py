import os
from copy import copy
from subprocess import run

from invoke import task

from faasmcli.util.env import PROJ_ROOT


@task
def start(ctx, workers=2):
    """
    Start the local dev cluster
    """
    env = copy(os.environ)
    env["FAASM_BUILD_MOUNT"] = "/build/faasm"
    env["FAASM_LOCAL_MOUNT"] = "/usr/local/faasm"

    cmd = [
        "docker-compose up -d",
        "--scale worker={}".format(workers),
        "nginx",
    ]
    cmd = " ".join(cmd)
    print(cmd)
    run(cmd, shell=True, check=True, cwd=PROJ_ROOT)


@task
def await_upload(ctx, host, port=8002):
    """
    Wait for local upload server
    """
    run(
        "bin/wait_for_upload.sh {} {}".format(host, port),
        shell=True,
        check=True,
        cwd=PROJ_ROOT,
    )


@task
def stop(ctx, workers=2):
    """
    Stop the local dev cluster
    """
    run("docker-compose stop", shell=True, check=True, cwd=PROJ_ROOT)


@task
def restart(ctx):
    """
    Restarts the whole dev cluster
    """
    run("docker-compose restart", shell=True, check=True, cwd=PROJ_ROOT)


@task
def restart_worker(ctx):
    """
    Restarts the workers in the dev cluster
    """
    run("docker-compose restart worker", shell=True, check=True, cwd=PROJ_ROOT)


@task
def logs(ctx):
    """
    Follows the logs of the dev cluster
    """
    run("docker-compose logs -f", shell=True, check=True, cwd=PROJ_ROOT)


@task
def flush_redis(ctx):
    """
    Flush redis in the dev cluster
    """
    for r in ["redis-state", "redis-queue"]:
        run(
            "docker-compose exec {} redis-cli flushall".format(r),
            shell=True,
            check=True,
            cwd=PROJ_ROOT,
        )
