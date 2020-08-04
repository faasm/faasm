import os
import docker
from packaging import version

from copy import copy
from os.path import join, exists
from subprocess import call, check_output

import jinja2
from invoke import task

from faasmcli.util.env import PROJ_ROOT
from faasmcli.util.version import get_faasm_version

# Order matters here
RELEASE_CONTAINERS = [
    "base",
    "base-test",
    "redis",
    "worker",
    "upload",
    "knative-worker",
    "toolchain",
]

ALL_CONTAINERS = [
    "cpp-root",
]
ALL_CONTAINERS += RELEASE_CONTAINERS


@task
def purge(context):
    """
    Purge docker images
    """
    images_cmd = ["docker", "images", "-q", "-f", "dangling=true"]
    image_list = check_output(images_cmd)

    for img in image_list.decode().split("\n"):
        if not img.strip():
            continue

        print("Removing {}".format(img))
        cmd = ["docker", "rmi", "-f", img]
        call(cmd)


def _check_valid_containers(containers):
    for container in containers:
        dockerfile = join(PROJ_ROOT, "docker", "{}.dockerfile".format(container))

        if not exists(dockerfile):
            print("Could not find dockerfile at {}".format(dockerfile))
            raise RuntimeError("Invalid container: {}".format(container))

        return dockerfile


def _do_push(container, version):
    res = call("docker push faasm/{}:{}".format(container, version), shell=True, cwd=PROJ_ROOT)
    if res != 0:
        raise RuntimeError("Failed docker push for {}:{}".format(container, version))


@task
def release(ctx, nocache=False):
    """
    Build containers for a release
    """
    build(ctx, RELEASE_CONTAINERS, nocache=nocache, push=True)


@task(iterable=["c"])
def build(ctx, c, nocache=False, push=False):
    """
    Build container images
    """

    # -----------------------
    # NOTE - to allow container-specific dockerignore files, we need to switch on DOCKER_BUILDKIT=1
    # this might change in future:
    #
    # https://stackoverflow.com/questions/40904409/how-to-specify-different-dockerignore-files-for-different-builds-in-the-same-pr
    # https://github.com/moby/moby/issues/12886#issuecomment-480575928
    # -----------------------

    shell_env = copy(os.environ)
    shell_env["DOCKER_BUILDKIT"] = "1"

    _check_valid_containers(c)

    faasm_ver = get_faasm_version()

    for container in c:
        # Check if we need to template a special dockerignore file
        # It seems the dockerignore file needs to be at <dockerfile_path>.dockerignore
        dockerfile = join("docker", "{}.dockerfile".format(container))
        dockerignore_template_path = join("docker", "{}.dockerignore.j2".format(container))
        dockerignore_target_path = "{}.dockerignore".format(dockerfile)

        if exists(join(PROJ_ROOT, dockerignore_template_path)):
            print("Templating new dockerignore file for {}".format(container))

            jinja_env = jinja2.Environment(loader=jinja2.FileSystemLoader(PROJ_ROOT))
            template = jinja_env.get_template(dockerignore_template_path)
            with open(join(PROJ_ROOT, dockerignore_target_path), "w") as fh:
                fh.write(template.render())

        tag_name = "faasm/{}:{}".format(container, faasm_ver)

        if nocache:
            no_cache_str = "--no-cache"
        else:
            no_cache_str = ""

        faasm_ver = get_faasm_version()

        cmd = "docker build {} -t {} --build-arg FAASM_VERSION={} -f {} .".format(no_cache_str, tag_name, faasm_ver,
                                                                                  dockerfile)
        print(cmd)
        res = call(cmd, shell=True, cwd=PROJ_ROOT, env=shell_env)
        if res != 0:
            raise RuntimeError("Failed docker build for {}".format(tag_name))

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
        res = call("docker pull faasm/{}:{}".format(container, faasm_ver), shell=True, cwd=PROJ_ROOT)
        if res != 0:
            raise RuntimeError("Failed docker pull for {}".format(container))


@task
def pull_release(ctx):
    """
    Pull container images for a release
    """
    pull(ctx, RELEASE_CONTAINERS)


@task
def push_release(ctx):
    """
    Push container images for a release
    """

    push(ctx, RELEASE_CONTAINERS)


@task
def build_all(ctx, nocache=False, push=False):
    build(ctx, ALL_CONTAINERS, nocache, push)


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
