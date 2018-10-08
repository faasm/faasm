from subprocess import call

from invoke import task

from tasks.env import PROJ_ROOT


@task
def start_all(context):
    call("docker-compose up -d", shell=True, cwd=PROJ_ROOT)


@task
def stop_all(context):
    call("docker-compose stop", shell=True, cwd=PROJ_ROOT)


@task
def restart_all(context):
    call("docker-compose stop", shell=True, cwd=PROJ_ROOT)
    call("docker-compose up -d", shell=True, cwd=PROJ_ROOT)
