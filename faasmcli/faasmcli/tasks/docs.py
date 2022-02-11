from os import makedirs
from os.path import join
from subprocess import run

from faasmcli.util.env import DOCS_ROOT

from invoke import task

SPHINX_OUT_DIR = join(DOCS_ROOT, "sphinx")


@task(default=True)
def generate(ctx):
    """
    Generates the docs
    """
    makedirs(SPHINX_OUT_DIR, exist_ok=True)

    run(
        "sphinx-build -b html {} {}".format(DOCS_ROOT, SPHINX_OUT_DIR),
        cwd=DOCS_ROOT,
        check=True,
        shell=True,
    )
