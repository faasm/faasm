import os
import docker
from packaging import version

from copy import copy
from os.path import join, exists
from subprocess import run, PIPE

from invoke import task

from faasmcli.util.env import PROJ_ROOT
from faasmcli.util.version import get_faasm_version


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
    for container in containers:
        dockerfile = join(
            PROJ_ROOT, "docker", "{}.dockerfile".format(container)
        )

        if not exists(dockerfile):
            print("Could not find dockerfile at {}".format(dockerfile))
            raise RuntimeError("Invalid container: {}".format(container))

        return dockerfile


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

    faasm_ver = get_faasm_version()

    for container in c:
        dockerfile = join("docker", "{}.dockerfile".format(container))
        tag_name = "faasm/{}:{}".format(container, faasm_ver)

        faasm_ver = get_faasm_version()

        cmd = "docker build {} -t {} --build-arg FAASM_VERSION={} -f {} .".format(
            "--no-cache" if nocache else "", tag_name, faasm_ver, dockerfile
        )
        print(cmd)
        run(cmd, shell=True, check=True, cwd=PROJ_ROOT, env=shell_env)

        if push:
            _do_push(container, faasm_ver)


@task(iterable=["c"])
def push(ctx, c):
    """
    Push container images
    """
    faasm_ver = get_faasm_version()

    _check_valid_containers(c)

    for container in c:
        _do_push(container, faasm_ver)


@task(iterable=["c"])
def pull(ctx, c):
    """
    Pull container images
    """
    faasm_ver = get_faasm_version()

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
    faasm_ver = get_faasm_version()

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
