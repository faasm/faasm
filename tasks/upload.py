import multiprocessing
import os
from os.path import join

from invoke import task

from tasks.util.env import FUNC_BUILD_DIR, PROJ_ROOT, RUNTIME_S3_BUCKET, FUNC_DIR, WASM_DIR
from tasks.util.upload_util import curl_file, upload_file_to_s3, upload_file_to_ibm

DIRS_TO_INCLUDE = ["demo", "errors", "python", "polybench", "sgd", "tf"]

PYTHON_FUNC_DIR = join(FUNC_DIR, "python")


def _get_s3_key(user, func):
    s3_key = "wasm/{}/{}/function.wasm".format(user, func)
    return s3_key


@task
def upload(ctx, user, func, host="127.0.0.1",
           s3=False, ibm=False, subdir=None,
           py=False, ts=False, built=True):
    if py:
        func_file = join(PROJ_ROOT, "func", user, "{}.py".format(func))

        url = "http://{}:8002/p/{}/{}".format(host, user, func)
        curl_file(url, func_file)
    elif ts:
        func_file = join(PROJ_ROOT, "typescript", "build", "{}.wasm".format(func))
        url = "http://{}:8002/f/ts/{}".format(host, func)
        curl_file(url, func_file)
    else:
        base_dir = FUNC_BUILD_DIR if built else WASM_DIR

        if subdir:
            func_file = join(base_dir, user, subdir, "{}.wasm".format(func))
        else:
            func_file = join(base_dir, user, "{}.wasm".format(func))

        if s3:
            print("Uploading {}/{} to S3".format(user, func))
            s3_key = _get_s3_key(user, func)
            upload_file_to_s3(func_file, RUNTIME_S3_BUCKET, s3_key)
        if ibm:
            print("Uploading {}/{} to IBM cloud storage".format(user, func))
            ibm_key = _get_s3_key(user, func)
            upload_file_to_ibm(func_file, RUNTIME_S3_BUCKET, ibm_key)
        else:
            url = "http://{}:8002/f/{}/{}".format(host, user, func)
            curl_file(url, func_file)


def _do_upload_all(host=None, upload_s3=False, py=False, built=True):
    to_upload = []

    if py:
        dir_to_walk = FUNC_DIR
    elif built:
        dir_to_walk = FUNC_BUILD_DIR
    else:
        dir_to_walk = WASM_DIR

    extension = ".py" if py else ".wasm"
    url_part = "p" if py else "f"

    if upload_s3 and py:
        raise RuntimeError("Not yet implemented python and S3 upload")

    # Walk the function directory tree
    for root, dirs, files in os.walk(dir_to_walk):
        # Strip original dir from root
        rel_path = root.replace(dir_to_walk, "")
        rel_path = rel_path.strip("/")

        path_parts = rel_path.split("/")
        if not path_parts:
            continue

        if path_parts[0] not in DIRS_TO_INCLUDE:
            continue

        user = path_parts[0]

        for f in files:
            if f.endswith(extension):
                func = f.replace(extension, "")
                func_file = join(root, f)

                if upload_s3:
                    print("Uploading {}/{} to S3".format(user, func))
                    s3_key = _get_s3_key(user, func)
                    upload_file_to_s3(func_file, RUNTIME_S3_BUCKET, s3_key)
                else:
                    print("Uploading {}/{} to host {}".format(user, func, host))
                    url = "http://{}:8002/{}/{}/{}".format(host, url_part, user, func)
                    to_upload.append((url, func_file))

    # Pool of uploaders
    p = multiprocessing.Pool(multiprocessing.cpu_count() - 1)
    p.starmap(curl_file, to_upload)


@task
def upload_all(ctx, host="127.0.0.1", py=False):
    _do_upload_all(host=host, py=py)


@task
def upload_all_s3(ctx):
    _do_upload_all(upload_s3=True)
