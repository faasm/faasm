from invoke import task

from tasks.env import playbook_command


@task
def k8_setup(context):
    playbook_command("kubernetes.yml", inventory="lsds.ini")


@task
def k8_deploy(context):
    playbook_command("deploy.yml", inventory="lsds.ini")


@task
def k8_clean(context):
    playbook_command("clean.yml", inventory="lsds.ini")
