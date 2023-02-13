import os
import docker
from packaging import version

from copy import copy
from os.path import join
from subprocess import run, PIPE

from invoke import task

from faasmcli.util.env import (
    FAASM_SGX_MODE_DISABLED,
    FAASM_SGX_MODE_HARDWARE,
    FAASM_SGX_MODE_SIM,
    PROJ_ROOT,
)
from faasmcli.util.version import get_version

SGX_HW_CONTAINER_SUFFIX = "-sgx"
SGX_SIMULATION_CONTAINER_SUFFIX = "-sgx-sim"
CONTAINER_NAME2FILE_MAP = {
    "redis": "redis.dockerfile",
    "minio": "minio.dockerfile",
    "cpp-root": "cpp-root.dockerfile",
    "base": "base.dockerfile",
    "base-sgx": "base-sgx.dockerfile",
    "base-sgx-sim": "base-sgx.dockerfile",
    "upload": "upload.dockerfile",
    "worker": "worker.dockerfile",
    "worker-sgx": "worker.dockerfile",
    "worker-sgx-sim": "worker.dockerfile",
    "cli": "cli.dockerfile",
    "cli-sgx": "cli.dockerfile",
    "cli-sgx-sim": "cli.dockerfile",
    "sgx-aesmd": "sgx-aesmd.dockerfile",
}


@task
def purge(context):
    """
    Purge docker images
    """
    images_cmd = ["docker", "images", "-q", "-f", "dangling=true"]
    cmd_out = run(images_cmd, stdout=PIPE, stderr=PIPE, check=True)
    image_list = cmd_out.stdout

    for img in image_list.decode().split("\n"):
        if not img.strip():
            continue

        print("Removing {}".format(img))
        cmd = ["docker", "rmi", "-f", img]
        run(cmd, check=True)


def _check_valid_containers(containers):
    for container_name in containers:
        if container_name not in CONTAINER_NAME2FILE_MAP:
            print(
                "Could not find dockerfile for container: {}".format(
                    container_name
                )
            )
            raise RuntimeError("Invalid container: {}".format(container_name))


def _do_push(container, version):
    run(
        "docker push faasm/{}:{}".format(container, version),
        shell=True,
        cwd=PROJ_ROOT,
        check=True,
    )


@task(iterable=["c"])
def build(ctx, c, nocache=False, push=False):
    """
    Build latest version of container images
    """
    # Use buildkit for nicer logging
    shell_env = copy(os.environ)
    shell_env["DOCKER_BUILDKIT"] = "1"

    _check_valid_containers(c)

    faasm_ver = get_version()

    for container_name in c:
        # Prepare dockerfile and tag name
        dockerfile = join("docker", CONTAINER_NAME2FILE_MAP[container_name])
        tag_name = "faasm/{}:{}".format(container_name, faasm_ver)

        # Prepare build arguments
        build_args = {"FAASM_VERSION": faasm_ver}
        if container_name.endswith(SGX_HW_CONTAINER_SUFFIX):
            build_args["FAASM_SGX_MODE"] = FAASM_SGX_MODE_HARDWARE
            build_args["FAASM_SGX_PARENT_SUFFIX"] = SGX_HW_CONTAINER_SUFFIX
        elif container_name.endswith(SGX_SIMULATION_CONTAINER_SUFFIX):
            build_args["FAASM_SGX_MODE"] = FAASM_SGX_MODE_SIM
            build_args[
                "FAASM_SGX_PARENT_SUFFIX"
            ] = SGX_SIMULATION_CONTAINER_SUFFIX
        else:
            build_args["FAASM_SGX_MODE"] = FAASM_SGX_MODE_DISABLED

        # Prepare docker command
        cmd = [
            "docker build {}".format("--no-cache" if nocache else ""),
            "-t {}".format(tag_name),
            "{}".format(
                " ".join(
                    [
                        "--build-arg {}={}".format(arg, build_args[arg])
                        for arg in build_args
                    ]
                )
            ),
            "-f {} .".format(dockerfile),
        ]
        docker_cmd = " ".join(cmd)
        print(docker_cmd)

        # Build (and push) docker image
        run(docker_cmd, shell=True, check=True, cwd=PROJ_ROOT, env=shell_env)
        if push:
            _do_push(container_name, faasm_ver)


@task
def build_all(ctx, nocache=False, push=False):
    """
    Build all available containers
    """
    build(ctx, [c for c in CONTAINER_NAME2FILE_MAP], nocache, push)


@task(iterable=["c"])
def push(ctx, c):
    """
    Push container images
    """
    faasm_ver = get_version()

    _check_valid_containers(c)

    for container in c:
        _do_push(container, faasm_ver)


@task(iterable=["c"])
def pull(ctx, c):
    """
    Pull container images
    """
    faasm_ver = get_version()

    _check_valid_containers(c)

    for container in c:
        run(
            "docker pull faasm/{}:{}".format(container, faasm_ver),
            shell=True,
            check=True,
            cwd=PROJ_ROOT,
        )


@task
def delete_old(ctx):
    """
    Deletes old Docker images
    """
    faasm_ver = get_version()

    dock = docker.from_env()
    images = dock.images.list()
    for image in images:
        for t in image.tags:
            if not t.startswith("faasm/"):
                continue

            tag_ver = t.split(":")[-1]
            if version.parse(tag_ver) < version.parse(faasm_ver):
                print("Removing old image: {}".format(t))
                dock.images.remove(t, force=True)
