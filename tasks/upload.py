from os.path import join
from subprocess import call

from invoke import task

from tasks.env import PROJ_ROOT


@task
def upload(context, user, func_name):
    print("Uploading {} for user {}".format(func_name, user))

    # Put function file in place
    wasm_file = join(PROJ_ROOT, "work", "out.wasm")

    # Run upload script
    upload_script = join(PROJ_ROOT, "cmake-build-debug", "bin", "offline")
    upload_cmd = [
        upload_script, wasm_file, user, func_name
    ]

    res = call(" ".join(upload_cmd), shell=True, cwd=PROJ_ROOT)

    if res == 0:
        print("Upload finished")
    else:
        print("Upload failed")
