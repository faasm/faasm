import os
from os.path import join

import boto3
from invoke import task

from tasks.env import AWS_REGION, RUNTIME_S3_BUCKET, WASM_FUNC_BUILD_DIR, EMSCRIPTEN_FUNC_BUILD_DIR
from tasks.upload_util import curl_file

DIRS_TO_INCLUDE = ["demo", "errors", "sgd"]


@task
def upload_func(ctx, user, func, host="localhost", emscripten=False):
    func_dir = EMSCRIPTEN_FUNC_BUILD_DIR if emscripten else WASM_FUNC_BUILD_DIR

    func_file = join(func_dir, user, "{}.wasm".format(func))
    url = "http://{}:8002/f/{}/{}".format(host, user, func)
    curl_file(url, func_file)


@task
def upload_funcs(ctx, host="localhost", emscripten=False):
    func_dir = EMSCRIPTEN_FUNC_BUILD_DIR if emscripten else WASM_FUNC_BUILD_DIR

    # Walk the function directory tree
    for root, dirs, files in os.walk(func_dir):
        # Strip original dir from root
        rel_path = root.replace(func_dir, "")
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
                print("Uploading {}/{} to host {}".format(user, func, host))

                url = "http://{}:8002/f/{}/{}".format(host, user, func)
                curl_file(url, func_file)
