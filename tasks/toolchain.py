from os.path import join, exists
from subprocess import call, check_output

from invoke import task

from tasks.env import PYODIDE_ROOT, MISC_S3_BUCKET
from tasks.upload_util import upload_file_to_s3, download_file_from_s3

EMSDK_BASE_DIR = join(PYODIDE_ROOT, "emsdk")
EMSDK_TAR_NAME = "emsdk.tar.gz"
EMSDK_TAR_PATH = join(EMSDK_BASE_DIR, EMSDK_TAR_NAME)


def _call(cmd):
    check_output(cmd, cwd=EMSDK_BASE_DIR, shell=True)


@task
def backup_emsdk(ctx):
    # Compress
    print("Creating archive of emsdk")
    _call("tar -cf {} emsdk".format(EMSDK_TAR_NAME))

    # Upload
    print("Uploading archive to S3")
    file_path = join(EMSDK_BASE_DIR, EMSDK_TAR_NAME)
    upload_file_to_s3(file_path, MISC_S3_BUCKET, EMSDK_TAR_NAME)

    # Remove old tar
    print("Removing archive")
    _call("rm {}".format(EMSDK_TAR_NAME))


@task
def restore_emsdk(ctx):
    # Nuke existing emsdk
    if exists(EMSDK_BASE_DIR):
        print("Deleting existing emsdk")
        _call("rm -rf {}".format(EMSDK_BASE_DIR))

    check_output("mkdir -p {}".format(EMSDK_BASE_DIR), shell=True)

    # Download (don't use boto in case needs to be done anonymously
    print("Downloading archive")
    download_file_from_s3(MISC_S3_BUCKET, EMSDK_TAR_NAME, EMSDK_TAR_PATH, boto=False)

    # Extract
    print("Extracting archive")
    _call("tar -xf {}".format(EMSDK_TAR_NAME))

    print("Removing archive")
    _call("rm {}".format(EMSDK_TAR_NAME))
