from os.path import join, exists
from subprocess import call, check_output

from invoke import task

from tasks.env import PYODIDE_ROOT, MISC_S3_BUCKET
from tasks.upload_util import upload_file_to_s3, download_file_from_s3

FAASM_LOCAL_DIR = "/usr/local/faasm"
EMSDK_BASE_DIR = join(FAASM_LOCAL_DIR, "emsdk")
EMSDK_TAR_NAME = "emsdk.tar.gz"
EMSDK_TAR_PATH = join(FAASM_LOCAL_DIR, EMSDK_TAR_NAME)


@task
def backup_emsdk(ctx):
    # Compress
    print("Creating archive of emsdk")
    check_output("tar -cf {} emsdk".format(EMSDK_TAR_PATH), shell=True)

    # Upload
    print("Uploading archive to S3")
    upload_file_to_s3(EMSDK_TAR_PATH, MISC_S3_BUCKET, EMSDK_TAR_NAME)

    # Remove old tar
    print("Removing archive")
    check_output("rm {}".format(EMSDK_TAR_PATH))


@task
def restore_emsdk(ctx):
    # Nuke existing emsdk
    if exists(EMSDK_BASE_DIR):
        print("Deleting existing emsdk")
        check_output("rm -rf {}".format(EMSDK_BASE_DIR), shell=True)

    # Download (don't use boto in case needs to be done anonymously
    print("Downloading archive")
    download_file_from_s3(MISC_S3_BUCKET, EMSDK_TAR_NAME, EMSDK_TAR_PATH, boto=False)

    # Extract
    print("Extracting archive")
    check_output("tar -xf {}".format(EMSDK_TAR_NAME), cwd=FAASM_LOCAL_DIR, shell=True)

    print("Removing archive")
    check_output("rm {}".format(EMSDK_TAR_PATH), shell=True)
