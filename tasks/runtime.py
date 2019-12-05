from os import remove
from os.path import exists, join
from shutil import rmtree
from subprocess import check_output

from invoke import task

from tasks.python import run_python_codegen
from tasks.util.env import FAASM_RUNTIME_ROOT, FAASM_LOCAL_DIR, MISC_S3_BUCKET
from tasks.util.upload_util import upload_file_to_s3, download_tar_from_s3
from tasks.util.version import get_faasm_version


def _get_tar_name():
    ver = get_faasm_version()
    return "faasm-runtime-root-{}.tar.gz".format(ver)


def _get_tar_path():
    tar_name = _get_tar_name()
    return join(FAASM_LOCAL_DIR, tar_name)


@task
def backup_runtime_root(ctx):
    tar_name = _get_tar_name()
    tar_path = _get_tar_path()

    # Compress
    print("Creating archive of Faasm runtime root")
    check_output("tar -cf {} runtime_root".format(tar_path), shell=True, cwd=FAASM_LOCAL_DIR)

    # Upload
    print("Uploading archive to S3")
    upload_file_to_s3(tar_path, MISC_S3_BUCKET, tar_name, public=True)

    # Remove old tar
    print("Removing archive")
    remove(tar_path)


@task
def download_runtime_root(ctx):
    tar_name = _get_tar_name()
    tar_path = _get_tar_path()

    # Clear out existing
    if exists(FAASM_RUNTIME_ROOT):
        print("Removing existing")
        rmtree(FAASM_RUNTIME_ROOT)

    # Download the bundle
    print("Downloading from S3")
    download_tar_from_s3(MISC_S3_BUCKET, tar_name, FAASM_LOCAL_DIR, boto=False)

    # Remove downloaded tar
    remove(tar_path)

    # Run codegen
    print("Running codegen")
    run_python_codegen(ctx)
