from copy import copy
from faasmcli.util.env import (
    FAASM_BUILD_DIR,
    FAASM_BUILD_DIR_DETACHED,
    FAASM_LOCAL_DIR_DETACHED,
    FAASM_SGX_MODE_DISABLED,
    FAASM_SGX_MODE_HARDWARE,
    FAASM_SGX_MODE_SIM,
    PROJ_ROOT,
)
from faasmcli.util.version import get_version
from faasmcli.util.env import AVAILABLE_HOSTS_SET
from faasmtools.build import FAASM_LOCAL_DIR
from invoke import task
from os import environ
from os.path import join
from subprocess import run


@task(optional=["detached"])
def start(ctx, workers=2, sgx=FAASM_SGX_MODE_DISABLED, detached=False):
    """
    Start the local dev cluster
    """
    # This env makes sure we mount our local setup into the containers, rather
    # than using the prebuilt binaries
    env = copy(environ)
    env["FAASM_BUILD_DIR"] = join(PROJ_ROOT, "dev/faasm/build")
    # In GHA we want to be able to pass a string argument to detached, yet
    # disable it. So passing "False" also disables the detached flag
    if detached and detached != "False":
        env["FAASM_BUILD_MOUNT"] = FAASM_BUILD_DIR_DETACHED
        env["FAASM_LOCAL_MOUNT"] = FAASM_LOCAL_DIR_DETACHED
    else:
        env["FAASM_BUILD_MOUNT"] = FAASM_BUILD_DIR
        env["FAASM_LOCAL_MOUNT"] = FAASM_LOCAL_DIR

    faasm_ver = get_version()
    if sgx == FAASM_SGX_MODE_SIM:
        env["FAASM_CLI_IMAGE"] = "faasm/cli-sgx-sim:{}".format(faasm_ver)
        env["FAASM_WORKER_IMAGE"] = "faasm/worker-sgx-sim:{}".format(faasm_ver)
        env["WASM_VM"] = "sgx"
    elif sgx == FAASM_SGX_MODE_HARDWARE:
        env["FAASM_CLI_IMAGE"] = "faasm/cli-sgx:{}".format(faasm_ver)
        env["FAASM_WORKER_IMAGE"] = "faasm/worker-sgx:{}".format(faasm_ver)
        env["WASM_VM"] = "sgx"

    cmd = [
        "docker compose up -d",
        "--scale worker={}".format(workers),
        "nginx",
    ]
    cmd = " ".join(cmd)
    print(cmd)

    run(cmd, shell=True, check=True, cwd=PROJ_ROOT, env=env)


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
    run(
        "docker compose stop nginx worker upload minio redis-state redis-queue",
        shell=True,
        check=True,
        cwd=PROJ_ROOT,
    )


@task
def restart(ctx):
    """
    Restart the whole dev cluster
    """
    run("docker compose restart", shell=True, check=True, cwd=PROJ_ROOT)


@task
def restart_worker(ctx):
    """
    Restart the workers in the dev cluster
    """
    run("docker compose restart worker", shell=True, check=True, cwd=PROJ_ROOT)


@task
def logs(ctx):
    """
    Follow the logs of the dev cluster
    """
    run("docker compose logs -f", shell=True, check=True, cwd=PROJ_ROOT)


@task
def flush_redis(ctx):
    """
    Flush Redis in the dev cluster
    """
    for r in ["redis-state", "redis-queue"]:
        run(
            "docker compose exec {} redis-cli flushall".format(r),
            shell=True,
            check=True,
            cwd=PROJ_ROOT,
        )


@task
def available_workers(ctx):
    """
    Return the list of available workers in Redis
    """
    run(
        "docker compose exec redis-queue redis-cli smembers {}".format(
            AVAILABLE_HOSTS_SET
        ),
        shell=True,
        check=True,
        cwd=PROJ_ROOT,
    )
