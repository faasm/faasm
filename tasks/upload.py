import multiprocessing
from functools import partial
from os import makedirs, listdir
from os.path import join, exists
from shutil import copy, rmtree

from invoke import task

from tasks.util.endpoints import get_upload_host_port
from tasks.util.env import PROJ_ROOT, RUNTIME_S3_BUCKET, FUNC_DIR, WASM_DIR, FAASM_SHARED_STORAGE_ROOT, \
    FAASM_SHARED_ROOT, FAASM_RUNTIME_ROOT
from tasks.util.genomics import INDEX_CHUNKS
from tasks.util.upload_util import curl_file, upload_file_to_s3, upload_file_to_ibm

PYTHON_FUNC_DIR = join(FUNC_DIR, "python")


def _get_s3_key(user, func):
    s3_key = "wasm/{}/{}/function.wasm".format(user, func)
    return s3_key


def _upload_function(user, func, port=None, host=None, s3=False, ibm=False, py=False, ts=False, file=None,
                     local_copy=False):
    host, port = get_upload_host_port(host, port)

    if py and local_copy:
        storage_dir = join(FAASM_SHARED_STORAGE_ROOT, "pyfuncs", user, func)
        runtime_dir = join(FAASM_RUNTIME_ROOT, "pyfuncs", user, func)

        if exists(runtime_dir):
            rmtree(runtime_dir)

        if not exists(storage_dir):
            makedirs(storage_dir)

        src_file = join(FUNC_DIR, user, "{}.py".format(func))
        dest_file = join(storage_dir, "function.py")
        copy(src_file, dest_file)
    elif py:
        func_file = join(PROJ_ROOT, "func", user, "{}.py".format(func))

        url = "http://{}:{}/p/{}/{}".format(host, port, user, func)
        curl_file(url, func_file)
    elif ts:
        func_file = join(PROJ_ROOT, "typescript", "build", "{}.wasm".format(func))
        url = "http://{}:{}/f/ts/{}".format(host, port, func)
        curl_file(url, func_file)
    else:
        if file:
            func_file = file
        else:
            func_file = join(WASM_DIR, user, func, "function.wasm")

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


@task
def user(ctx, user, host=None, py=False, local_copy=False):
    """
    Upload all functions for the given user
    """
    if py:
        # Get all Python funcs
        funcs = listdir(join(FUNC_DIR, user))
        funcs = [f for f in funcs if f.endswith(".py")]
        funcs = [f.replace(".py", "") for f in funcs]
    else:
        funcs = listdir(join(WASM_DIR, user))

        # Filter in only functions that exist
        funcs = [f for f in funcs if exists(join(WASM_DIR, user, f, "function.wasm"))]

    upload_partial = partial(_upload_function, user, host=host, py=py, local_copy=local_copy)
    p = multiprocessing.Pool(multiprocessing.cpu_count() - 1)
    p.starmap(upload_partial, [(f,) for f in funcs])


@task(default=True)
def upload(ctx, user, func, host=None, s3=False, ibm=False, py=False, ts=False, file=None, local_copy=False):
    """
    Upload a function
    """
    _upload_function(user, func, host=host, s3=s3, ibm=ibm, py=py, ts=ts, file=file, local_copy=local_copy)


@task
def genomics(ctx, host="localhost", port=None):
    """
    Upload all the genomics functions
    """

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
