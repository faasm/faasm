from os import makedirs, walk
from os.path import join
from subprocess import call

from invoke import task

from faasmcli.util.endpoints import get_upload_host_port
from faasmcli.util.env import FUNC_DIR, FAASM_SHARED_STORAGE_ROOT
from faasmcli.util.state import upload_binary_state, upload_shared_file


@task
def state(ctx, host=None):
    """
    Upload Tensorflow lite demo state
    """
    data_dir = join(FUNC_DIR, "tf", "data")
    model_file = "mobilenet_v1_1.0_224.tflite"
    host, _ = get_upload_host_port(host, None)
    file_path = join(data_dir, model_file)

    upload_binary_state("tf", "mobilenet_v1", file_path, host=host)


@task
def upload(ctx, host=None, local_copy=False):
    """
    Upload Tensorflow lite demo data
    """
    host, port = get_upload_host_port(host, None)

    source_data = join(FUNC_DIR, "tf", "data")

    dest_root = join(FAASM_SHARED_STORAGE_ROOT, "tfdata")
    if local_copy:
        makedirs(dest_root, exist_ok=True)

    for root, dirs, files in walk(source_data):
        for filename in files:
            file_path = join(source_data, filename)

            if local_copy:
                dest_file = join(dest_root, filename)
                call("cp {} {}".format(file_path, dest_file), shell=True)
            else:
                shared_path = "tfdata/{}".format(filename)
                upload_shared_file(host, file_path, shared_path)
