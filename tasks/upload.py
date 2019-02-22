import os
from os.path import join, exists
from subprocess import call

import boto3
from invoke import task

from tasks.env import WASM_DIR, AWS_REGION, RUNTIME_S3_BUCKET

DIRS_TO_SKIP = ["errors", "dummy", "python"]


@task
def upload_funcs(context, s3=False, host="localhost"):
    s3_client = None
    if s3:
        s3_client = boto3.resource('s3', region_name=AWS_REGION)

    # Walk the function directory tree
    for root, dirs, files in os.walk(WASM_DIR):
        # Strip original dir from root
        rel_path = root.replace(WASM_DIR, "")
        rel_path = rel_path.strip("/")

        path_parts = rel_path.split("/")
        if len(path_parts) != 2:
            continue

        if path_parts[0] in DIRS_TO_SKIP:
            continue

        user = path_parts[0]
        func = path_parts[1]
        wasm_file = join(root, "function.wasm")

        if not exists(wasm_file):
            print("Skipping {}/{}, couldn't find wasm file at {}".format(user, func, wasm_file))
            continue

        if s3:
            print("Uploading {}/{} to S3".format(user, func))

            s3_key = "wasm/{}/{}/function.wasm".format(user, func)

            s3_client.meta.client.upload_file(wasm_file, RUNTIME_S3_BUCKET, s3_key)

        else:
            print("Uploading {}/{} to host {}".format(user, func, host))

            cmd = [
                "curl",
                "-X", "PUT",
                "http://{}:8002/f/{}/{}".format(host, user, func),
                "-T", wasm_file
            ]

            cmd = " ".join(cmd)

            res = call(cmd, shell=True)

            if res == 0:
                print("Upload finished")
            else:
                print("Upload failed")
