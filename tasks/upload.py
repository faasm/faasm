from os.path import join
from subprocess import call

from invoke import task

from tasks.env import PROJ_ROOT


@task
def upload(context, user, func_name, host="localhost"):
    print("Uploading {} for user {}".format(func_name, user))

    wasm_file = join(PROJ_ROOT, "wasm", user, func_name, "function.wasm")

    cmd = [
        "curl",
        "-X", "PUT",
        "http://{}:8002/f/{}/{}".format(host, user, func_name),
        "-T", wasm_file
    ]

    cmd = " ".join(cmd)

    res = call(cmd, shell=True)

    if res == 0:
        print("Upload finished")
    else:
        print("Upload failed")
