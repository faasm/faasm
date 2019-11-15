from os import walk, makedirs
from os.path import join, exists
from shutil import rmtree
from subprocess import call
from subprocess import check_output

from invoke import task

from tasks.aws import invoke_lambda
from tasks.util.env import FUNC_DIR, FAASM_SHARED_STORAGE_ROOT
from tasks.util.env import STATE_S3_BUCKET, DATA_S3_BUCKET, FAASM_DATA_DIR
from tasks.util.kubernetes import get_kubernetes_upload_host
from tasks.util.matrices import RESULT_MATRIX_KEY, SUBMATRICES_KEY_A, SUBMATRICES_KEY_B, MATRIX_CONF_STATE_KEY
from tasks.util.matrices import get_params_file, get_mat_a_file, get_mat_b_file, get_result_file
from tasks.util.state import upload_binary_state, upload_sparse_matrix
from tasks.util.state import upload_shared_file
from tasks.util.upload_util import upload_file_to_s3, download_file_from_s3

_GENOMICS_TAR_NAME = "genomics.tar.gz"
_GENOMICS_TAR_PATH = "/tmp/{}".format(_GENOMICS_TAR_NAME)
_GENOMICS_TAR_DIR_NAME = "genomics"
_GENOMICS_DATA_DIR = join(FAASM_DATA_DIR, _GENOMICS_TAR_DIR_NAME)

_REUTERS_TAR_NAME = "reuters.tar.gz"
_REUTERS_TAR_PATH = "/tmp/{}".format(_REUTERS_TAR_NAME)
_REUTERS_TAR_DIR_NAME = "reuters"
_REUTERS_DATA_DIR = join(FAASM_DATA_DIR, _REUTERS_TAR_DIR_NAME)

_MATRIX_TAR_NAME = "matrix.tar.gz"
_MATRIX_TAR_PATH = "/tmp/{}".format(_MATRIX_TAR_NAME)
_MATRIX_TAR_DIR_NAME = "matrix"
_MATRIX_DATA_DIR = join(FAASM_DATA_DIR, _MATRIX_TAR_DIR_NAME)

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
# S3 UPLOAD/ DOWNLOAD
# -------------------------------------------------

@task
def reuters_upload_s3(ctx):
    _do_s3_upload(_REUTERS_TAR_PATH, _REUTERS_TAR_DIR_NAME, _REUTERS_TAR_NAME)


@task
def matrix_upload_s3(ctx):
    _do_s3_upload(_MATRIX_TAR_PATH, _MATRIX_TAR_DIR_NAME, _MATRIX_TAR_NAME)


@task
def genomics_upload_s3(ctx):
    _do_s3_upload(_GENOMICS_TAR_PATH, _GENOMICS_TAR_DIR_NAME, _GENOMICS_TAR_NAME)


def _do_s3_upload(tar_path, tar_dir, tar_name):
    # Compress
    print("Creating archive of data {}".format(tar_path))
    check_output("tar -cf {} {}".format(tar_path, tar_dir), shell=True, cwd=FAASM_DATA_DIR)

    # Upload
    print("Uploading archive to S3")
    upload_file_to_s3(tar_path, DATA_S3_BUCKET, tar_name)

    # Remove old tar
    print("Removing archive")
    check_output("rm {}".format(tar_path), shell=True)


@task
def reuters_download_s3(ctx):
    _do_s3_download(_REUTERS_TAR_PATH, _REUTERS_TAR_DIR_NAME, _REUTERS_TAR_NAME)


@task
def matrix_download_s3(ctx):
    _do_s3_download(_MATRIX_TAR_PATH, _MATRIX_TAR_DIR_NAME, _MATRIX_TAR_NAME)


@task
def genomics_download_s3(ctx):
    _do_s3_download(_GENOMICS_TAR_PATH, _GENOMICS_TAR_DIR_NAME, _GENOMICS_TAR_NAME)


def _do_s3_download(tar_path, tar_dir, tar_name):
    # Clear out existing
    print("Removing existing {}".format(tar_dir))
    full_tar_dir = join(FAASM_DATA_DIR, tar_dir)
    if exists(full_tar_dir):
        rmtree(full_tar_dir)

    # Download the bundle
    print("Downloading from S3 to {}".format(tar_path))
    download_file_from_s3(DATA_S3_BUCKET, tar_name, tar_path)

    # Extract
    print("Extracting")
    check_output("tar -xf {}".format(tar_path), shell=True, cwd=FAASM_DATA_DIR)


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


def _do_reuters_upload(host=None, s3_bucket=None, knative=False):
    assert host or s3_bucket or knative, "Must give a host, S3 bucket or knative"
    user = "sgd"

    host = get_kubernetes_upload_host(knative, host)

    # Upload the matrix data
    upload_sparse_matrix(user, "inputs", _REUTERS_DATA_DIR, host=host, s3_bucket=s3_bucket)

    # Upload the categories data
    cat_path = join(_REUTERS_DATA_DIR, "outputs")
    upload_binary_state(user, "outputs", cat_path, host=host, s3_bucket=s3_bucket)

    # Upload the feature counts
    counts_path = join(_REUTERS_DATA_DIR, "feature_counts")
    upload_binary_state(user, "feature_counts", counts_path, host=host, s3_bucket=s3_bucket)


# -------------------------------------------------
# MATRIX UPLOAD
# -------------------------------------------------

@task
def matrix_state_upload(ctx, mat_size, n_splits, host=None, knative=True):
    user = "python"

    host = get_kubernetes_upload_host(knative, host)

    upload_binary_state(user, MATRIX_CONF_STATE_KEY, get_params_file(mat_size, n_splits), host=host)

    upload_binary_state(user, SUBMATRICES_KEY_A, get_mat_a_file(mat_size, n_splits), host=host)
    upload_binary_state(user, SUBMATRICES_KEY_B, get_mat_b_file(mat_size, n_splits), host=host)

    upload_binary_state(user, RESULT_MATRIX_KEY, get_result_file(mat_size, n_splits), host=host)


# -------------------------------------------------
# TENSORFLOW UPLOAD
# -------------------------------------------------

@task
def tf_upload_data(ctx, host="localhost", local_copy=False):
    source_data = join(FUNC_DIR, "tf", "data")

    dest_root = join(FAASM_SHARED_STORAGE_ROOT, "tfdata")
    if local_copy and not exists(dest_root):
        makedirs(dest_root)

    for root, dirs, files in walk(source_data):
        for filename in files:
            file_path = join(source_data, filename)

            if local_copy:
                dest_file = join(dest_root, filename)
                call("cp {} {}".format(file_path, dest_file), shell=True)
            else:
                shared_path = "tfdata/{}".format(filename)
                upload_shared_file(host, file_path, shared_path)


# -------------------------------------------------
# GENOMICS UPLOAD
# -------------------------------------------------

@task
def genomics_upload_data(ctx, host="localhost", local_copy=False):
    dest_root = join(FAASM_SHARED_STORAGE_ROOT, "genomics")
    if local_copy and not exists(dest_root):
        makedirs(dest_root)

    read_copies = 5

    # Create list of files as tuples of (source, dest)
    reads_source_file = "reads_1.fq"
    files = list()
    for idx in range(0, read_copies):
        files.append((reads_source_file, "reads_{}.fq".format(idx)) for idx in range(0, read_copies))

    files.append(("human_c_20_idx.gem.gem", "human_c_20_idx.gem.gem"))

    for f in files:
        file_path = join(FAASM_DATA_DIR, "genomics", f)

        if local_copy:
            dest_file = join(dest_root, f)
            call("cp {} {}".format(file_path, dest_file), shell=True)
        else:
            shared_path = "genomics/{}".format(f)
            upload_shared_file(host, file_path, shared_path)
