from os import makedirs
from os.path import join, exists
from shutil import rmtree
from subprocess import run

from faasmcli.util.env import DOCS_ROOT

from invoke import task

SPHINX_OUT_DIR = join(DOCS_ROOT, "sphinx")

GENERATED_DIRS = ["apidoc", "sphinx", "doxygen"]


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


@task(default=True)
def clean(ctx):
    """
    Removes all generated docs files
    """
    for d in GENERATED_DIRS:
        dir_path = join(DOCS_ROOT, d)
        if exists(dir_path):
            rmtree(dir_path)
