from subprocess import call

from invoke import task

from tasks.util.env import PROJ_ROOT


@task
def start(ctx):
    call("docker-compose up -d", shell=True, cwd=PROJ_ROOT)


@task
def stop(ctx):
    call("docker-compose stop", shell=True, cwd=PROJ_ROOT)


@task
def restart(ctx):
    call("docker-compose stop", shell=True, cwd=PROJ_ROOT)
    call("docker-compose up -d", shell=True, cwd=PROJ_ROOT)
