from os import makedirs, listdir
from os.path import join, exists
from shutil import copy

from invoke import task

from tasks.util.endpoints import get_upload_host_port
from tasks.util.env import PROJ_ROOT, RUNTIME_S3_BUCKET, FUNC_DIR, WASM_DIR, FAASM_SHARED_STORAGE_ROOT
from tasks.util.genomics import INDEX_CHUNKS
from tasks.util.upload_util import curl_file, upload_file_to_s3, upload_file_to_ibm

DIRS_TO_INCLUDE = ["demo", "errors", "omp", "mpi", "python", "polybench", "sgd", "tf", "gene"]

PYTHON_FUNC_DIR = join(FUNC_DIR, "python")


def _get_s3_key(user, func):
    s3_key = "wasm/{}/{}/function.wasm".format(user, func)
    return s3_key


def _upload_function(user, func, port=None, host=None, s3=False, ibm=False, py=False, ts=False, file=None,
                     local_copy=False):
    host, port = get_upload_host_port(host, port)

    if py and local_copy:
        storage_dir = join(FAASM_SHARED_STORAGE_ROOT, "pyfuncs", user, func)
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
def upload_user(ctx, user, host=None, py=False, local_copy=False):
    if py:
        funcs = listdir(join(FUNC_DIR, user))
        funcs = [f for f in funcs if f.endswith(".py")]
        funcs = [f.replace(".py", "") for f in funcs]
    else:
        funcs = listdir(join(WASM_DIR, user))

    # TODO - use multiprocessing to avoid this being super slow
    for func in funcs:
        _upload_function(user, func, host=host, py=py, local_copy=local_copy)


@task
def upload(ctx, user, func, host=None, s3=False, ibm=False, py=False, ts=False, file=None, local_copy=False):
    _upload_function(user, func, host=host, s3=s3, ibm=ibm, py=py, ts=ts, file=file, local_copy=local_copy)


@task
def upload_genomics(ctx, host="localhost", port=None):
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
