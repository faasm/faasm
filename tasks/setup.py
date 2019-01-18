from invoke import task

from tasks.env import playbook_command, sudo_script


# -------------------------------
# Regular runtime set-up
# -------------------------------

@task
def setup_local(context):
    setup_namespaces(context)
    setup_net_files(context)
    setup_cgroup(context)


@task
def setup_namespaces(context):
    sudo_script("netns.sh 20")


@task
def setup_net_files(context):
    playbook_command("net_files.yml")


@task
def setup_cgroup(context):
    sudo_script("cgroup.sh")


# -------------------------------
# One-off installs
# -------------------------------

@task
def setup_libs(context):
    playbook_command("libs.yml")


@task
def setup_protobuf(context):
    playbook_command("protobuf.yml")


@task
def setup_pistache(context):
    playbook_command("pistache.yml")


@task
def setup_clang(context):
    playbook_command("clang.yml")


@task
def setup_spdlog(context):
    playbook_command("spdlog.yml")


@task
def setup_eigen(context):
    playbook_command("eigen.yml")
