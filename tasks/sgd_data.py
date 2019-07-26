from os.path import join
from shutil import rmtree
from subprocess import check_output

from invoke import task

from tasks.aws import invoke_lambda
from tasks.util.env import HOME_DIR, STATE_S3_BUCKET, DATA_S3_BUCKET
from tasks.util.upload_util import curl_file, upload_file_to_s3, download_file_from_s3

_DATA_TAR_NAME = "reuters.tar.gz"
_DATA_TAR_PATH = "/tmp/{}".format(_DATA_TAR_NAME)
_DATA_DIR = join(HOME_DIR, "faasm", "data")
_TAR_DIR_NAME = "reuters"
_FULL_DATA_DIR = join(_DATA_DIR, _TAR_DIR_NAME)


_ALL_REUTERS_STATE_KEYS = [
    "feature_counts",
    "inputs_innr",
    "inputs_nonz",
    "inputs_outr",
    "inputs_size",
    "inputs_vals",
    "outputs",
]

_SPARSE_MATRIX_PARTS = [
    "vals", "innr", "outr", "size", "nonz"
]


# -------------------------------------------------
# RAW DATA
# -------------------------------------------------

@task
def reuters_upload_s3(ctx):
    """
    Uploads Reuters data having been parsed from the original Hogwild dataset
    """
    # Compress
    print("Creating archive of Reuters data")
    check_output("tar -cf {} {}".format(_DATA_TAR_PATH, _TAR_DIR_NAME), shell=True, cwd=_DATA_DIR)

    # Upload
    print("Uploading archive to S3")
    upload_file_to_s3(_DATA_TAR_PATH, DATA_S3_BUCKET, _DATA_TAR_NAME)

    # Remove old tar
    print("Removing archive")
    check_output("rm {}".format(_DATA_TAR_PATH), shell=True)


@task
def reuters_download_s3(ctx):
    """
    Downloads Reuters data
    # """
    # Clear out existing
    print("Removing existing")
    rmtree(join(_DATA_DIR, _TAR_DIR_NAME))

    # Download the bundle
    print("Downloading from S3")
    download_file_from_s3(DATA_S3_BUCKET, _DATA_TAR_NAME, _DATA_TAR_PATH)

    # Extract
    print("Extracting")
    check_output("tar -xf {}".format(_DATA_TAR_PATH), shell=True, cwd=_DATA_DIR)


# -------------------------------------------------
# STATE MANAGEMENT
# -------------------------------------------------

@task
def reuters_state_upload(ctx, host):
    _do_reuters_upload(host=host)


@task
def reuters_state_upload_s3(ctx):
    _do_reuters_upload(s3_bucket=STATE_S3_BUCKET)


@task
def reuters_prepare_aws(ctx):
    for state_key in _ALL_REUTERS_STATE_KEYS:
        # Note, hack here where we pass state key as "function"
        invoke_lambda(ctx, "faasm-state", payload={
            "user": "sgd",
            "function": state_key,
        })


def _do_reuters_upload(host=None, s3_bucket=None):
    assert host or s3_bucket, "Must give a host or S3 bucket"
    user = "sgd"

    # Upload the matrix data
    _upload_sparse_matrix(user, "inputs", _FULL_DATA_DIR, host=host, s3_bucket=s3_bucket)

    # Upload the categories data
    cat_path = join(_FULL_DATA_DIR, "outputs")
    _upload_binary_file(user, "outputs", cat_path, host=host, s3_bucket=s3_bucket)

    # Upload the feature counts
    counts_path = join(_FULL_DATA_DIR, "feature_counts")
    _upload_binary_file(user, "feature_counts", counts_path, host=host, s3_bucket=s3_bucket)


def _upload_sparse_matrix(user, key, directory, host=None, s3_bucket=None):
    for f in _SPARSE_MATRIX_PARTS:
        this_key = "{}_{}".format(key, f)
        file_path = join(directory, f)

        _upload_binary_file(user, this_key, file_path, host=host, s3_bucket=s3_bucket)


def _upload_binary_file(user, key, binary_file, host=None, s3_bucket=None):
    print("Uploading binary file at {} for user {}".format(binary_file, user))

    if s3_bucket:
        s3_key = "{}/{}".format(user, key)
        print("Uploading matrix binary to S3 {} -> {}/{}".format(key, s3_bucket, s3_key))
        upload_file_to_s3(binary_file, s3_bucket, s3_key)
    else:
        url = "http://{}:8002/s/{}/{}".format(host, user, key)
        curl_file(url, binary_file)
