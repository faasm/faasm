from invoke import task

from tasks.env import playbook_command


@task
def setup_libs(context):
    playbook_command("libs.yml")


@task
def setup_network(context):
    playbook_command("network.yml")


@task
def setup_cgroup(context):
    playbook_command("cgroup.yml")


@task
def setup_cgroup(context):
    playbook_command("protobuf.yml")


@task
def setup_clang(context):
    playbook_command("clang.yml")
