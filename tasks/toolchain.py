from os import remove
from os.path import join, exists
from subprocess import check_output

from invoke import task

from tasks.util.env import MISC_S3_BUCKET, TOOLCHAIN_ROOT, FAASM_LOCAL_DIR, FAASM_SYSROOT
from tasks.util.upload_util import upload_file_to_s3, download_file_from_s3

TOOLCHAIN_INSTALL = join(TOOLCHAIN_ROOT, "install")
TOOLCHAIN_TAR_NAME = "faasm-toolchain.tar.gz"
TOOLCHAIN_TAR_PATH = join(TOOLCHAIN_ROOT, TOOLCHAIN_TAR_NAME)

SYSROOT_TAR_NAME = "faasm-sysroot.tar.gz"
SYSROOT_TAR_PATH = join(FAASM_LOCAL_DIR, SYSROOT_TAR_NAME)


@task
def backup_toolchain(ctx):
    print("Creating archives of Faasm toolchain and sysroot")
    check_output("tar -cf {} install".format(TOOLCHAIN_TAR_NAME), shell=True, cwd=TOOLCHAIN_ROOT)
    check_output("tar -cf {} llvm-sysroot".format(SYSROOT_TAR_NAME), shell=True, cwd=FAASM_LOCAL_DIR)

    # Upload
    print("Uploading archives to S3")
    upload_file_to_s3(TOOLCHAIN_TAR_PATH, MISC_S3_BUCKET, TOOLCHAIN_TAR_NAME)
    upload_file_to_s3(SYSROOT_TAR_PATH, MISC_S3_BUCKET, SYSROOT_TAR_NAME)

    # Remove old tar
    print("Removing archives")
    remove(TOOLCHAIN_TAR_PATH)
    remove(SYSROOT_TAR_PATH)

@task
def download_toolchain(ctx):
    # Nuke existing toolchain and sysroot
    if exists(TOOLCHAIN_INSTALL):
        print("Deleting existing toolchain at {}".format(TOOLCHAIN_INSTALL))
        check_output("rm -rf {}".format(TOOLCHAIN_INSTALL), shell=True)

    if exists(FAASM_SYSROOT):
        print("Deleting existing sysroot at {}".format(FAASM_SYSROOT))
        check_output("rm -rf {}".format(FAASM_SYSROOT), shell=True)

    # Download (note not using Boto)
    print("Downloading archives")
    download_file_from_s3(MISC_S3_BUCKET, TOOLCHAIN_TAR_NAME, TOOLCHAIN_TAR_PATH, boto=False)
    download_file_from_s3(MISC_S3_BUCKET, SYSROOT_TAR_NAME, SYSROOT_TAR_PATH, boto=False)

    # Extract
    print("Extracting archives")
    check_output("tar -xf {}".format(TOOLCHAIN_TAR_NAME), cwd=TOOLCHAIN_ROOT, shell=True)
    check_output("tar -xf {}".format(SYSROOT_TAR_NAME), cwd=FAASM_LOCAL_DIR, shell=True)

    print("Removing archives")
    remove(TOOLCHAIN_TAR_PATH)
    remove(SYSROOT_TAR_PATH)
