from os.path import join, exists
from subprocess import call

from invoke import task

from tasks.env import PROJ_ROOT


@task
def upload_binary_file(ctx, host, user, key, binary_file):
    print("Uploading binary file at {} for user {}".format(binary_file, user))

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
def upload_sparse_matrix(ctx, host, user, key, directory):
    files = ["vals", "innr", "outr", "size", "nonz"]

    for f in files:
        this_key = "{}_{}".format(key, f)
        file_path = join(directory, f)
        print("Uploading matrix binary to {} from {}".format(this_key, file_path))
        upload_binary_file(ctx, host, user, this_key, file_path)


@task
def reuters_upload(ctx, host, user):
    directory = "/tmp/reuters_out/"
    user = "sgd"

    # Upload the matrix data
    upload_sparse_matrix(ctx, host, user, "inputs", directory)

    # Upload the categories data
    cat_path = join(directory, "outputs")
    upload_binary_file(ctx, host, user, "outputs", cat_path)
