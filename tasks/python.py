from os import remove
from os.path import join, exists
from subprocess import call

from invoke import task

from tasks.env import PROJ_ROOT, PYODIDE_ROOT


@task
def python_codegen(ctx):
    codegen_bin = join(PROJ_ROOT, "cmake-build-debug", "bin", "codegen")

    with open(join(PROJ_ROOT, "python", "python_shared_obj.txt")) as fh:
        for so_file in fh:
            res = call("{} {}".format(codegen_bin, so_file), shell=True)
            if res != 0:
                print("Failed to generate machine code for {}".format(so_file))

