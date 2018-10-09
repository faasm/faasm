from invoke import task

from tasks.env import playbook_command


@task
def setup_libs(context):
    playbook_command("libs.yml")


@task
def setup_namespaces(context):
    playbook_command("namespaces.yml")


@task
def setup_net_files(context):
    playbook_command("net_files.yml")


@task
def setup_cgroup(context):
    playbook_command("cgroup.yml")


@task
def setup_protobuf(context):
    playbook_command("protobuf.yml")


@task
def setup_clang(context):
    playbook_command("clang.yml")
