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
    call("docker build -t shillaker/faasm-base -f base.dockerfile .", shell=True, cwd=PROJ_ROOT)


@task
def build_core(context):
    call("docker build -t shillaker/faasm-core .", shell=True, cwd=PROJ_ROOT)


@task
def push_core(context):
    call("docker push shillaker/faasm-core", shell=True, cwd=PROJ_ROOT)
