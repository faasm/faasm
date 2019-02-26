import os
from os.path import join, exists
from subprocess import call

import boto3
from invoke import task

from tasks.env import AWS_REGION, RUNTIME_S3_BUCKET, PROJ_ROOT

DIRS_TO_INCLUDE = ["demo", "errors", "sgd"]

WASM_FUNC_BUILD_DIR = join(PROJ_ROOT, "func", "wasm_func_build")


@task
def upload_funcs(context, s3=False, host="localhost"):
    s3_client = None
    if s3:
        s3_client = boto3.resource('s3', region_name=AWS_REGION)

    # Walk the function directory tree
    for root, dirs, files in os.walk(WASM_FUNC_BUILD_DIR):
        # Strip original dir from root
        rel_path = root.replace(WASM_FUNC_BUILD_DIR, "")
        rel_path = rel_path.strip("/")

        path_parts = rel_path.split("/")
        if len(path_parts) != 1:
            continue

        if path_parts[0] not in DIRS_TO_INCLUDE:
            continue

        user = path_parts[0]

        for f in files:
            if f.endswith(".wasm"):
                func = f.replace(".wasm", "")
                func_file = join(root, f)
                if s3:
                    print("Uploading {}/{} to S3".format(user, func))

                    s3_key = "wasm/{}/{}/function.wasm".format(user, func)

                    s3_client.meta.client.upload_file(func_file, RUNTIME_S3_BUCKET, s3_key)

                else:
                    print("Uploading {}/{} to host {}".format(user, func, host))

                    cmd = [
                        "curl",
                        "-X", "PUT",
                        "http://{}:8002/f/{}/{}".format(host, user, func),
                        "-T", func_file
                    ]

                    cmd = " ".join(cmd)

                    res = call(cmd, shell=True)

                    if res == 0:
                        print("Upload finished")
                    else:
                        print("Upload failed")
