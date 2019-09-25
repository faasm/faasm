from os import remove
from os.path import join, exists
from subprocess import check_output

from invoke import task

from tasks.util.env import MISC_S3_BUCKET, TOOLCHAIN_ROOT
from tasks.util.upload_util import upload_file_to_s3, download_file_from_s3

TOOLCHAIN_INSTALL = join(TOOLCHAIN_ROOT, "install")
TAR_NAME = "faasm-toolchain.tar.gz"
TAR_PATH = join(TOOLCHAIN_ROOT, TAR_NAME)


@task
def backup_toolchain(ctx):
    # Compress
    print("Creating archive of Faasm toolchain")
    check_output("tar -cf {} install".format(TAR_NAME), shell=True, cwd=TOOLCHAIN_ROOT)

    # Upload
    print("Uploading archive to S3")
    upload_file_to_s3(TAR_PATH, MISC_S3_BUCKET, TAR_NAME)

    # Remove old tar
    print("Removing archive")
    remove(TAR_PATH)


@task
def download_toolchain(ctx):
    # Nuke existing toolchain
    if exists(TOOLCHAIN_INSTALL):
        print("Deleting existing toolchain at {}".format(TOOLCHAIN_INSTALL))
        check_output("rm -rf {}".format(TOOLCHAIN_INSTALL), shell=True)

    # Download (note not using Boto)
    print("Downloading archive")
    download_file_from_s3(MISC_S3_BUCKET, TAR_NAME, TAR_PATH, boto=False)

    # Extract
    print("Extracting archive")
    check_output("tar -xf {}".format(TAR_NAME), cwd=TOOLCHAIN_ROOT, shell=True)

    print("Removing archive")
    remove(TAR_PATH)
