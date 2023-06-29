import os
from os.path import join
from copy import copy
from subprocess import run

from invoke import task

from faasmcli.util.env import (
    FAASM_SGX_MODE_DISABLED,
    FAASM_SGX_MODE_HARDWARE,
    FAASM_SGX_MODE_SIM,
    PROJ_ROOT,
)
from faasmcli.util.version import get_version


def _get_cluster_services():
    compose_cmd = "docker compose ps --services"
    service_list = (
        run(compose_cmd, shell=True, cwd=PROJ_ROOT, capture_output=True)
        .stdout.decode("utf-8")
        .split("\n")
    )
    service_list = [
        service
        for service in service_list
        if len(service) > 0 and service not in ["faasm-cli", "cpp", "python"]
    ]
    return service_list


@task
def start(ctx, workers=2, sgx=FAASM_SGX_MODE_DISABLED):
    """
    Start the local dev cluster
    """
    # This env makes sure we mount our local setup into the containers, rather
    # than using the prebuilt binaries
    env = copy(os.environ)
    env["FAASM_BUILD_DIR"] = join(PROJ_ROOT, "dev/faasm/build")
    env["FAASM_BUILD_MOUNT"] = "/build/faasm"
    env["FAASM_LOCAL_MOUNT"] = "/usr/local/faasm"
    env["PLANNER_BUILD_MOUNT"] = env["FAASM_BUILD_MOUNT"]

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
        "docker compose stop {}".format(" ".join(_get_cluster_services())),
        shell=True,
        check=True,
        cwd=PROJ_ROOT,
    )


@task
def restart(ctx):
    """
    Restart the whole dev cluster
    """
    run(
        "docker compose restart {}".format(" ".join(_get_cluster_services())),
        shell=True,
        check=True,
        cwd=PROJ_ROOT,
    )


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
