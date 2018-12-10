from subprocess import call

from invoke import task

from tasks.env import PROJ_ROOT, HOME_DIR

@task
def pull(context):
    images = [
        "faasm/worker",
        "faasm/edge",
        "faasm/upload",
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
def data(context):
    cmd = [
        "docker",
        "run",
        "-v {}:/root".format(HOME_DIR),
        "-v {}:/work".format(PROJ_ROOT),
        "-w /work",
        "-it",
        "--net=host",
        "faasm/data",
        "/bin/bash"
    ]
    cmd = " ".join(cmd)

    call(cmd, shell=True, cwd=PROJ_ROOT)


@task
def build_all(context):
    build_base(context)

    build_worker(context)
    build_edge(context)
    build_upload(context)
    build_data(context)

    build_toolchain(context)


@task
def push_all(context):
    push_base(context)

    push_worker(context)
    push_edge(context)
    push_upload(context)
    push_data(context)

    push_toolchain(context)


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
def push_base(context):
    call("docker push faasm/base", shell=True, cwd=PROJ_ROOT)


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


@task
def build_upload(context):
    call("docker build -t faasm/upload  -f docker/upload.dockerfile .", shell=True, cwd=PROJ_ROOT)


@task
def push_upload(context):
    call("docker push faasm/upload", shell=True, cwd=PROJ_ROOT)


@task
def build_data(context):
    call("docker build -t faasm/data  -f docker/data.dockerfile .", shell=True, cwd=PROJ_ROOT)


@task
def push_data(context):
    call("docker push faasm/data", shell=True, cwd=PROJ_ROOT)
