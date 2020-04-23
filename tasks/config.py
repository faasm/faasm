from invoke import task

from tasks.util.config import get_faasm_config


@task
def create(ctx):
    """
    Set up skeleton Faasm config
    """
    get_faasm_config()
