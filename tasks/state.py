from os.path import join, exists
from subprocess import call

from invoke import task

from tasks.env import PROJ_ROOT


def _upload_binary_file(host, user, key, binary_file):
    cmd = [
        "curl",
        "-X", "PUT",
        "http://{}:8002/s/{}/{}".format(host, user, key),
        "-T", binary_file
    ]

    cmd = " ".join(cmd)

    res = call(cmd, shell=True)

    if res == 0:
        print("Upload finished")
    else:
        print("Upload failed")


@task
def upload_sparse_matrix(ctx, user, directory):
    print("Uploading matrix binaries at {} for user {}".format(directory, user))
    host = "localhost"

    files = ["vals", "innr", "outr", "size", "nonz"]

    for f in files:
        file_path = join(directory, f)
        print("Uploading matrix binary at {}".format(file_path))
        _upload_binary_file(host, user, f, file_path)
