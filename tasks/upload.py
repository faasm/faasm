import multiprocessing
import os
from os import makedirs
from os.path import join, exists
from subprocess import call

from invoke import task

from tasks.util.endpoints import get_upload_host_port
from tasks.util.env import FUNC_BUILD_DIR, PROJ_ROOT, RUNTIME_S3_BUCKET, FUNC_DIR, WASM_DIR, FAASM_SHARED_STORAGE_ROOT
from tasks.util.genomics import INDEX_CHUNKS
from tasks.util.upload_util import curl_file, upload_file_to_s3, upload_file_to_ibm

DIRS_TO_INCLUDE = ["demo", "errors", "python", "polybench", "sgd", "tf", "gene"]

PYTHON_FUNC_DIR = join(FUNC_DIR, "python")


def _get_s3_key(user, func):
    s3_key = "wasm/{}/{}/function.wasm".format(user, func)
    return s3_key


@task
def upload(ctx, user, func, host=None, s3=False, ibm=False, py=False, ts=False, prebuilt=False):
    host, port = get_upload_host_port(host, None)

    if py:
        func_file = join(PROJ_ROOT, "func", user, "{}.py".format(func))

        url = "http://{}:{}/p/{}/{}".format(host, port, user, func)
        curl_file(url, func_file)
    elif ts:
        func_file = join(PROJ_ROOT, "typescript", "build", "{}.wasm".format(func))
        url = "http://{}:{}/f/ts/{}".format(host, port, func)
        curl_file(url, func_file)
    else:
        base_dir = WASM_DIR if prebuilt else FUNC_BUILD_DIR

        if prebuilt:
            func_file = join(base_dir, user, func, "function.wasm")
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
            url = "http://{}:{}/f/{}/{}".format(host, port, user, func)
            curl_file(url, func_file)


def _do_upload_all(host=None, port=None, upload_s3=False, py=False, prebuilt=False, local_copy=False):
    to_upload = []

    if py:
        dir_to_walk = FUNC_DIR
    else:
        dir_to_walk = WASM_DIR if prebuilt else FUNC_BUILD_DIR

    extension = ".py" if py else ".wasm"
    url_part = "p" if py else "f"

    if upload_s3 and py:
        raise RuntimeError("Not yet implemented python and S3 upload")

    if local_copy and not py:
        raise RuntimeError("Not yet implemented local copy for non-python")
    else:
        storage_dir = join(FAASM_SHARED_STORAGE_ROOT, "pyfuncs")
        if not exists(storage_dir):
            makedirs(storage_dir)

    # Walk the function directory tree
    for root, dirs, files in os.walk(dir_to_walk):
        # Strip original dir from root
        rel_path = root.replace(dir_to_walk, "")
        rel_path = rel_path.strip("/")

        path_parts = rel_path.split("/")
        path_parts = [p for p in path_parts if p]
        if not path_parts:
            continue

        if path_parts[0] not in DIRS_TO_INCLUDE:
            continue

        user = path_parts[0]

        for f in files:
            if f.endswith(extension):
                if prebuilt:
                    func = path_parts[1]
                else:
                    func = f.replace(extension, "")

                func_file = join(root, f)

                if upload_s3:
                    print("Uploading {}/{} to S3".format(user, func))
                    s3_key = _get_s3_key(user, func)
                    upload_file_to_s3(func_file, RUNTIME_S3_BUCKET, s3_key)
                elif local_copy:
                    # Copy files directly into place
                    func_storage_dir = join(storage_dir, user, func)
                    if not exists(func_storage_dir):
                        makedirs(func_storage_dir)

                    dest_file = join(func_storage_dir, "function.py")
                    call("cp {} {}".format(func_file, dest_file), shell=True)
                else:
                    print("Uploading {}/{} to host {}".format(user, func, host))
                    url = "http://{}:{}/{}/{}/{}".format(host, port, url_part, user, func)
                    to_upload.append((url, func_file))

    # Drop out if already done local copy
    if local_copy:
        return

    # Pool of uploaders
    p = multiprocessing.Pool(multiprocessing.cpu_count() - 1)
    p.starmap(curl_file, to_upload)


@task
def upload_all(ctx, host=None, port=None, py=False, prebuilt=False, local_copy=False):
    host, port = get_upload_host_port(host, port)
    _do_upload_all(host=host, port=port, py=py, prebuilt=prebuilt, local_copy=local_copy)


@task
def upload_all_s3(ctx):
    _do_upload_all(upload_s3=True)


@task
def upload_genomics(ctx, host="localhost", port=8002):
    # When uploading genomics, we are uploading the mapper entrypoint as a normal
    # function, but the worker functions are all from the same source file

    # Upload the entrypoint function
    upload(ctx, "gene", "mapper")

    # Upload the worker functions (one for each index chunk)
    host, port = get_upload_host_port(host, port)

    for i in INDEX_CHUNKS:
        func_name = "mapper_index{}".format(i)
        print("Uploading function gene/{} to {}:{}".format(func_name, host, port))

        file_path = join(PROJ_ROOT, "third-party/gem3-mapper/wasm_bin/gem-mapper")
        url = "http://{}:{}/f/gene/{}".format(host, port, func_name)
        curl_file(url, file_path)
