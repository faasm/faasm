from os.path import join, exists
from subprocess import call, check_output

from invoke import task

from tasks.util.env import PROJ_ROOT
from tasks.util.version import get_faasm_version

# Order matters here
RELEASE_CONTAINERS = [
    "base",
    "worker",
    "upload",
    "knative-worker",
    "knative-native-base",
    "knative-native-python",
    "redis",
    "toolchain",
    "testing",
]


@task
def purge_images(context):
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
def docker_build_release(ctx, nocache=False):
    docker_build(ctx, RELEASE_CONTAINERS, nocache=nocache, push=True)


@task(iterable=["c"])
def docker_build(ctx, c, nocache=False, push=False):
    _check_valid_containers(c)

    version = get_faasm_version()

    for container in c:
        dockerfile = join(PROJ_ROOT, "docker", "{}.dockerfile".format(container))
        tag_name = "faasm/{}:{}".format(container, version)

        if nocache:
            no_cache_str = "--no-cache"
        else:
            no_cache_str = ""

        cmd = "docker build {} -t {} -f {} .".format(no_cache_str, tag_name, dockerfile)
        print(cmd)
        res = call(cmd, shell=True, cwd=PROJ_ROOT)
        if res != 0:
            raise RuntimeError("Failed docker build for {}".format(tag_name))

        if push:
            _do_push(container, version)


@task(iterable=["c"])
def docker_push(ctx, c):
    version = get_faasm_version()

    _check_valid_containers(c)

    for container in c:
        _do_push(container, version)


@task(iterable=["c"])
def docker_pull(ctx, c):
    version = get_faasm_version()

    _check_valid_containers(c)

    for container in c:
        res = call("docker pull faasm/{}:{}".format(container, version), shell=True, cwd=PROJ_ROOT)
        if res != 0:
            raise RuntimeError("Failed docker pull for {}".format(container))
