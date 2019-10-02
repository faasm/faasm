from os import makedirs
from os.path import exists, join
from subprocess import check_output

from invoke import task

from tasks.util.env import FUNC_DIR, FAASM_SHARED_STORAGE_ROOT


@task
def set_up_tensorflow_data(ctx):
    data_dir = join(FAASM_SHARED_STORAGE_ROOT, "tfdata")
    source_data = join(FUNC_DIR, "tf", "data")

    if not exists(data_dir):
        makedirs(data_dir)

    check_output("cp {}/* {}/".format(source_data, data_dir), shell=True)
