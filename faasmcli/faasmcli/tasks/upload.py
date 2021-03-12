from os import makedirs
from os.path import join, exists
from shutil import copy, rmtree

from invoke import task

from faasmcli.util.endpoints import get_upload_host_port
from faasmcli.util.env import (
    FAASM_SHARED_STORAGE_ROOT,
    FAASM_RUNTIME_ROOT,
)
from faasmcli.util.upload_util import curl_file


@task(default=True)
def upload(ctx, user, func, func_file, py=False, local_copy=False):
    """
    Upload a function
    """
    host, port = get_upload_host_port()

    if py and local_copy:
        storage_dir = join(FAASM_SHARED_STORAGE_ROOT, "pyfuncs", user, func)
        runtime_dir = join(FAASM_RUNTIME_ROOT, "pyfuncs", user, func)

        if exists(runtime_dir):
            rmtree(runtime_dir)

        if not exists(storage_dir):
            makedirs(storage_dir)

        dest_file = join(storage_dir, "function.py")
        copy(func_file, dest_file)
    elif py:
        url = "http://{}:{}/p/{}/{}".format(host, port, user, func)
        curl_file(url, func_file)
    else:
        url = "http://{}:{}/f/{}/{}".format(host, port, user, func)
        curl_file(url, func_file)
