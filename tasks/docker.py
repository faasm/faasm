from subprocess import call

from invoke import task

from tasks.env import PROJ_ROOT


@task
def tools(context):
    cmd = [
        "docker",
        "run",
        "-v {}:/work".format(PROJ_ROOT),
        "-w /work",
        "-it",
        "shillaker/wasm-toolchain",
        "/bin/bash"
    ]
    cmd = " ".join(cmd)

    call(cmd, shell=True, cwd=PROJ_ROOT)


@task
def build_base(context):
    call("docker build -t shillaker/faasm-base -f docker/base.dockerfile .", shell=True, cwd=PROJ_ROOT)


@task
def build_worker(context):
    call("docker build -t shillaker/faasm-worker  -f docker/worker.dockerfile .", shell=True, cwd=PROJ_ROOT)


@task
def push_worker(context):
    call("docker push shillaker/faasm-worker", shell=True, cwd=PROJ_ROOT)


@task
def build_edge(context):
    call("docker build -t shillaker/faasm-edge -f docker/edge.dockerfile .", shell=True, cwd=PROJ_ROOT)


@task
def push_edge(context):
    call("docker push shillaker/faasm-edge", shell=True, cwd=PROJ_ROOT)
