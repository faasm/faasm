from copy import copy
from invoke import task
from faasmcli.util.env import PROJ_ROOT
from faasmcli.util.shell import find_command
from faasmcli.util.version import get_faasm_version
from os import environ
from os.path import join
from subprocess import run


@task
def check(ctx):
    """
    Detect if SGX is supported
    """
    binary = find_command("detect_sgx")
    run(binary, shell=True)


@task
def build_worker_image(ctx, nocache=False):
    """
    Build worker with SGX enabled in hardware mode
    """
    # Use buildkit for nicer logging
    shell_env = copy(environ)
    shell_env["DOCKER_BUILDKIT"] = "1"

    containers = ["base", "worker"]
    faasm_ver = get_faasm_version()

    for container in containers:
        dockerfile = join("docker", "{}.dockerfile".format(container))
        tag_name = "faasm/{}-sgx:{}".format(container, faasm_ver)

        cmd = [
            "docker build {}".format("--no-cache" if nocache else ""),
            "-t {}".format(tag_name),
            "--build-arg FAASM_VERSION={}".format(faasm_ver),
            "--build-arg FAASM_SGX_MODE=Hardware",
            "-f {} .".format(dockerfile),
        ]
        docker_cmd = " ".join(cmd)
        print(docker_cmd)
        run(docker_cmd, shell=True, check=True, cwd=PROJ_ROOT, env=shell_env)

        docker_push_cmd = "docker push {}".format(tag_name)
        print(docker_push_cmd)
        run(docker_push_cmd, shell=True, check=True, cwd=PROJ_ROOT)
