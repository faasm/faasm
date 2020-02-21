import multiprocessing
import os
from os import makedirs
from os.path import join, exists
from subprocess import call

from invoke import task

from tasks import invoke
from tasks.util.endpoints import get_upload_host_port
from tasks.util.env import FUNC_BUILD_DIR, PROJ_ROOT, RUNTIME_S3_BUCKET, FUNC_DIR, WASM_DIR, FAASM_SHARED_STORAGE_ROOT
from tasks.util.genomics import INDEX_CHUNKS
from tasks.util.upload_util import curl_file, upload_file_to_s3, upload_file_to_ibm

# See the scripts in third-party/ParResKernels/scripts for the arguments they
# recommend for running the kernels.
ITERATIONS = 2
PRK_FUNCS = {
    "nstream": "{} 200 0".format(ITERATIONS),  # iterations, vector length, offset
}

FAASM_USER = "prk"


@task
def invoke_prk(ctx, func):
    if func not in PRK_FUNCS:
        print("Invalid PRK function {}".format(func))
        return 1

    invoke(ctx, FAASM_USER, func, cmdline=PRK_FUNCS[func])
