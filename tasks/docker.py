from subprocess import call

from invoke import task

from tasks.env import PROJ_ROOT


@task
def pull(context):
    images = [
        "faasm/worker",
        "faasm/edge",
        "faasm/toolchain",
    ]

    for image in images:
        cmd = [
            "docker",
            "pull",
            image
        ]

        cmd = " ".join(cmd)
        call(cmd, shell=True, cwd=PROJ_ROOT)


@task
def tools(context):
    cmd = [
        "docker",
        "run",
        "-v {}:/work".format(PROJ_ROOT),
        "-w /work",
        "-it",
        "--net=host",
        "faasm/toolchain",
        "/bin/bash"
    ]
    cmd = " ".join(cmd)

    call(cmd, shell=True, cwd=PROJ_ROOT)


@task
def build_toolchain(context):
    cmd = "docker build -t faasm/toolchain -f docker/toolchain.dockerfile ."
    print(cmd)
    call(cmd, shell=True, cwd=PROJ_ROOT)


@task
def push_toolchain(context):
    call("docker push faasm/toolchain", shell=True, cwd=PROJ_ROOT)


@task
def build_base(context):
    call("docker build -t faasm/base -f docker/base.dockerfile .", shell=True, cwd=PROJ_ROOT)


@task
def build_worker(context):
    call("docker build -t faasm/worker  -f docker/worker.dockerfile .", shell=True, cwd=PROJ_ROOT)


@task
def push_worker(context):
    call("docker push faasm/worker", shell=True, cwd=PROJ_ROOT)


@task
def build_edge(context):
    call("docker build -t faasm/edge -f docker/edge.dockerfile .", shell=True, cwd=PROJ_ROOT)


@task
def push_edge(context):
    call("docker push faasm/edge", shell=True, cwd=PROJ_ROOT)
