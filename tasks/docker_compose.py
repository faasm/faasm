from subprocess import call

from invoke import task

from tasks.util.env import PROJ_ROOT


@task
def docker_clear_queue(ctx):
    call("docker-compose exec redis-queue redis-cli flushall", shell=True, cwd=PROJ_ROOT)


@task
def docker_start_all(ctx):
    call("docker-compose up -d", shell=True, cwd=PROJ_ROOT)


@task
def docker_stop_all(ctx):
    call("docker-compose stop", shell=True, cwd=PROJ_ROOT)


@task
def docker_restart_all(ctx):
    call("docker-compose stop", shell=True, cwd=PROJ_ROOT)
    call("docker-compose up -d", shell=True, cwd=PROJ_ROOT)
