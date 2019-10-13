from os import walk
from os.path import join

from invoke import task

from tasks.util.env import FUNC_DIR
from tasks.util.state import upload_shared_file


@task
def set_up_tensorflow_data(ctx, host="localhost"):
    source_data = join(FUNC_DIR, "tf", "data")

    for root, dirs, files in walk(source_data):
        for filename in files:
            file_path = join(source_data, filename)
            shared_path = "tfdata/{}".format(filename)
            upload_shared_file(host, file_path, shared_path)
