from os import makedirs
from os.path import join, exists
from shutil import copy, rmtree

from invoke import task

from faasmcli.util.endpoints import get_upload_host_port
from faasmcli.util.env import (
    FAASM_SHARED_STORAGE_ROOT,
    FAASM_RUNTIME_ROOT,
    KEY_MANAGER_REGISTRY_IP,
    KEY_MANAGER_REGISTRY_PORT,
)
from faasmcli.util.upload_util import curl_file
from faasmcli.util.http import do_post


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
        if file:
            func_file = file
        else:
            func_file = join(WASM_DIR, user, func, "function.wasm")

        if func.endswith('sgx_wamr'):
            policy_file_path = join(WASM_DIR, user, func, "policy.json")
            with open(policy_file_path) as policy_file:
                policy = json.load(policy_file)
            url = "http://{}:{}/api/v1/registry/register/{}".format(KEY_MANAGER_REGISTRY_IP, KEY_MANAGER_REGISTRY_PORT, user)
            do_post(url, policy, json=True)

        url = "http://{}:{}/f/{}/{}".format(host, port, user, func)
        curl_file(url, func_file)
