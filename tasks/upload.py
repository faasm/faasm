import multiprocessing
import os
from os.path import join

from invoke import task

from tasks.util.env import FUNC_BUILD_DIR, PROJ_ROOT, RUNTIME_S3_BUCKET
from tasks.util.upload_util import curl_file, upload_file_to_s3

DIRS_TO_INCLUDE = ["demo", "errors", "onnx", "python", "polybench", "sgd"]


def _get_s3_key(user, func):
    s3_key = "wasm/{}/{}/function.wasm".format(user, func)
    return s3_key


@task
def upload(ctx, user, func, host="127.0.0.1", upload_s3=False, subdir=None):
    if subdir:
        func_file = join(FUNC_BUILD_DIR, user, subdir, "{}.wasm".format(func))
    else:
        func_file = join(FUNC_BUILD_DIR, user, "{}.wasm".format(func))

    if upload_s3:
        print("Uploading {}/{} to S3".format(user, func))
        s3_key = _get_s3_key(user, func)
        upload_file_to_s3(func_file, RUNTIME_S3_BUCKET, s3_key)
    else:
        url = "http://{}:8002/f/{}/{}".format(host, user, func)
        curl_file(url, func_file)


@task
def py_upload(ctx, user, func, host="127.0.0.1"):
    func_file = join(PROJ_ROOT, "func", user, "{}.py".format(func))

    url = "http://{}:8002/p/{}/{}".format(host, user, func)
    curl_file(url, func_file)


@task
def ts_upload(ctx, func, host="127.0.0.1"):
    func_file = join(PROJ_ROOT, "typescript", "build", "{}.wasm".format(func))
    url = "http://{}:8002/f/t/{}".format(host, func)
    curl_file(url, func_file)


def _do_upload_all(host=None, upload_s3=False):
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

                if upload_s3:
                    print("Uploading {}/{} to S3".format(user, func))
                    s3_key = _get_s3_key(user, func)
                    upload_file_to_s3(func_file, RUNTIME_S3_BUCKET, s3_key)
                else:
                    print("Uploading {}/{} to host {}".format(user, func, host))
                    url = "http://{}:8002/f/{}/{}".format(host, user, func)
                    to_upload.append((url, func_file))

    # Pool of uploaders
    p = multiprocessing.Pool(multiprocessing.cpu_count() - 1)
    p.starmap(curl_file, to_upload)


@task
def upload_all(ctx, host="127.0.0.1"):
    _do_upload_all(host=host)


@task
def upload_all_s3(ctx):
    _do_upload_all(upload_s3=True)
