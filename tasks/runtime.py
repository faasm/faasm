from os import makedirs, remove
from os.path import exists, join
from shutil import rmtree
from subprocess import check_output, call

from invoke import task

from tasks.python import set_up_python_runtime, run_python_codegen
from tasks.tensorflow import set_up_tensorflow_data
from tasks.util.env import FAASM_RUNTIME_ROOT, FAASM_LOCAL_DIR, MISC_S3_BUCKET, ANSIBLE_ROOT
from tasks.util.upload_util import upload_file_to_s3, download_tar_from_s3

RUNTIME_TAR_NAME = "faasm_runtime_root.tar.gz"
RUNTIME_TAR_PATH = "/tmp/{}".format(RUNTIME_TAR_NAME)

BACKUP_LOCATION = join(FAASM_LOCAL_DIR, "runtime_root_backup")


@task
def backup_runtime_root(ctx):
    # Nuke the existing runtime root
    if exists(FAASM_RUNTIME_ROOT):
        print("Moving existing runtime root to {}".format(BACKUP_LOCATION))
        if exists(BACKUP_LOCATION):
            rmtree(BACKUP_LOCATION)

        call("mv {} {}".format(FAASM_RUNTIME_ROOT, BACKUP_LOCATION), shell=True)

    print("Creating new runtime root dir")
    makedirs(FAASM_RUNTIME_ROOT)

    # Run the Ansible set-up script
    ret = call("ansible-playbook runtime_fs.yml", cwd=ANSIBLE_ROOT, shell=True)
    if ret != 0:
        print("Running ansible script failed")
        return 1

    # Set up the Python runtime
    set_up_python_runtime(ctx)

    # Set up tensorflow data
    set_up_tensorflow_data(ctx)

    # Create a tmp directory
    makedirs(join(FAASM_RUNTIME_ROOT, "tmp"))

    # Compress
    print("Creating archive of Faasm runtime root")
    check_output("tar -cf {} runtime_root".format(RUNTIME_TAR_PATH), shell=True, cwd=FAASM_LOCAL_DIR)

    # Upload
    print("Uploading archive to S3")
    upload_file_to_s3(RUNTIME_TAR_PATH, MISC_S3_BUCKET, RUNTIME_TAR_NAME, public=True)

    # Remove old tar
    print("Removing archive")
    remove(RUNTIME_TAR_PATH)


@task
def download_runtime_root(ctx):
    # Clear out existing
    if exists(FAASM_RUNTIME_ROOT):
        print("Removing existing")
        rmtree(FAASM_RUNTIME_ROOT)

    makedirs(FAASM_RUNTIME_ROOT)

    # Download the bundle
    print("Downloading from S3")
    download_tar_from_s3(MISC_S3_BUCKET, RUNTIME_TAR_NAME, FAASM_LOCAL_DIR, boto=False)

    # Run codegen
    print("Running codegen")
    run_python_codegen(ctx)
