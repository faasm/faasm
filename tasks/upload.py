import multiprocessing
import os
from os.path import join

from invoke import task

from tasks.env import FUNC_BUILD_DIR
from tasks.upload_util import curl_file

DIRS_TO_INCLUDE = ["demo", "errors", "sgd", "python", "polybench"]


@task
def upload_func(ctx, user, func, host="localhost"):
    func_dir = FUNC_BUILD_DIR

    func_file = join(func_dir, user, "{}.wasm".format(func))
    url = "http://{}:8002/f/{}/{}".format(host, user, func)
    curl_file(url, func_file)


@task
def upload_funcs(ctx, host="localhost"):
    func_dir = FUNC_BUILD_DIR

    to_upload = []

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
                to_upload.append((url, func_file))

    # Pool of uploaders
    p = multiprocessing.Pool(multiprocessing.cpu_count() - 1)
    p.starmap(curl_file, to_upload)
