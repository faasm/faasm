from os import walk, makedirs
from os.path import join, exists
from subprocess import call

from invoke import task

from tasks.util.env import FUNC_DIR, FAASM_SHARED_STORAGE_ROOT
from tasks.util.state import upload_shared_file


@task
def set_up_tensorflow_data(ctx, host="localhost", local_copy=False):
    source_data = join(FUNC_DIR, "tf", "data")

    dest_root = join(FAASM_SHARED_STORAGE_ROOT, "tfdata")
    if local_copy and not exists(dest_root):
        makedirs(dest_root)

    for root, dirs, files in walk(source_data):
        for filename in files:
            file_path = join(source_data, filename)

            if local_copy:
                dest_file = join(dest_root, filename)
                call("cp {} {}".format(file_path, dest_file), shell=True)
            else:
                shared_path = "tfdata/{}".format(filename)
                upload_shared_file(host, file_path, shared_path)
