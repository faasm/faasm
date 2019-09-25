from os import mkdir
from os.path import exists, join
from subprocess import call

from invoke import task

from tasks.util.env import FAASM_RUNTIME_ROOT, FUNC_DIR


@task
def set_up_tensorflow_data(ctx):
    data_dir = join(FAASM_RUNTIME_ROOT, "data")
    source_data = join(FUNC_DIR, "tf", "data")

    if not exists(data_dir):
        mkdir(data_dir)

    call("cp {}/* {}/".format(source_data, data_dir), shell=True)
