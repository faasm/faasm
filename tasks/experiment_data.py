from os.path import join
from shutil import rmtree
from subprocess import check_output

from invoke import task
from tasks.util.matrices import RESULT_MATRIX_KEY, SUBMATRICES_KEY_A, SUBMATRICES_KEY_B, MATRIX_CONF_STATE_KEY

from tasks.aws import invoke_lambda
from tasks.util.config import get_faasm_config
from tasks.util.env import HOME_DIR, STATE_S3_BUCKET, DATA_S3_BUCKET
from tasks.util.matrices import get_params_file, get_mat_a_file, get_mat_b_file, get_result_file
from tasks.util.state import upload_binary_state, upload_sparse_matrix
from tasks.util.upload_util import upload_file_to_s3, download_file_from_s3

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


# -------------------------------------------------
# REUTERS - S3
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
    """
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
# REUTERS UPLOAD
# -------------------------------------------------

@task
def reuters_state_upload(ctx, host=None, knative=True):
    _do_reuters_upload(host=host, knative=knative)


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


def _get_upload_host(knative, host):
    faasm_conf = get_faasm_config()
    if knative:
        if not faasm_conf.has_section("Kubernetes"):
            host = host if host else "localhost"
        else:
            host = faasm_conf["Kubernetes"]["upload_host"]

    return host


def _do_reuters_upload(host=None, s3_bucket=None, knative=False):
    assert host or s3_bucket or knative, "Must give a host, S3 bucket or knative"
    user = "sgd"

    host = _get_upload_host(knative, host)

    # Upload the matrix data
    upload_sparse_matrix(user, "inputs", _FULL_DATA_DIR, host=host, s3_bucket=s3_bucket)

    # Upload the categories data
    cat_path = join(_FULL_DATA_DIR, "outputs")
    upload_binary_state(user, "outputs", cat_path, host=host, s3_bucket=s3_bucket)

    # Upload the feature counts
    counts_path = join(_FULL_DATA_DIR, "feature_counts")
    upload_binary_state(user, "feature_counts", counts_path, host=host, s3_bucket=s3_bucket)


# -------------------------------------------------
# MATRIX UPLOAD
# -------------------------------------------------

@task
def matrix_state_upload(ctx, mat_size, n_splits, host=None, knative=True):
    user = "python"

    host = _get_upload_host(knative, host)

    upload_binary_state(user, MATRIX_CONF_STATE_KEY, get_params_file(mat_size, n_splits), host=host)

    upload_binary_state(user, SUBMATRICES_KEY_A, get_mat_a_file(mat_size, n_splits), host=host)
    upload_binary_state(user, SUBMATRICES_KEY_B, get_mat_b_file(mat_size, n_splits), host=host)

    upload_binary_state(user, RESULT_MATRIX_KEY, get_result_file(mat_size, n_splits), host=host)
