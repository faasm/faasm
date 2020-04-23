import multiprocessing
from os import walk, makedirs, listdir
from os.path import join, exists
from shutil import rmtree
from subprocess import call
from subprocess import check_output

from invoke import task

from tasks.util.endpoints import get_kubernetes_upload_host, get_upload_host_port
from tasks.util.env import FUNC_DIR, FAASM_SHARED_STORAGE_ROOT
from tasks.util.env import DATA_S3_BUCKET, FAASM_DATA_DIR
from tasks.util.matrices import get_matrix_dir
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
    """
    Upload data for the reuters experiment to S3
    """
    _do_s3_upload(_REUTERS_TAR_PATH, _REUTERS_TAR_DIR_NAME, _REUTERS_TAR_NAME)


@task
def matrix_upload_s3(ctx):
    """
    Upload data for the matrix experiment to S3
    """
    _do_s3_upload(_MATRIX_TAR_PATH, _MATRIX_TAR_DIR_NAME, _MATRIX_TAR_NAME)


@task
def genomics_upload_s3(ctx):
    """
    Upload data for the genomics experiment to S3
    """
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
    """
    Download data for the reuters experiment from S3
    """
    _do_s3_download(_REUTERS_TAR_PATH, _REUTERS_TAR_DIR_NAME, _REUTERS_TAR_NAME)


@task
def matrix_download_s3(ctx):
    """
    Download data for the matrix experiment from S3
    """
    _do_s3_download(_MATRIX_TAR_PATH, _MATRIX_TAR_DIR_NAME, _MATRIX_TAR_NAME)


@task
def genomics_download_s3(ctx):
    """
    Download data for the genomics experiment from S3
    """
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
def reuters_state(ctx, host=None, knative=True):
    """
    Upload reuters experiment state
    """
    _do_reuters_upload(host=host, knative=knative)


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

def _do_upload(data_dir, file_name, user, host, key=None):
    print("Uploading state {}".format(file_name))
    file_path = join(data_dir, file_name)

    key = key if key else file_name
    upload_binary_state(user, key, file_path, host=host)


@task
def matrix_state(ctx, mat_size, n_splits, host=None, knative=True):
    """
    Upload matrix experiment state
    """
    user = "python"

    host = get_kubernetes_upload_host(knative, host)
    data_dir = get_matrix_dir(mat_size, n_splits)
    file_names = listdir(data_dir)

    if len(file_names) == 0:
        print("Invalid matrix data: {} {}".format(mat_size, n_splits))
        exit(1)

    # Multiple uploaders as there may be lots of files
    args = [(data_dir, f, user, host) for f in file_names]
    p = multiprocessing.Pool(multiprocessing.cpu_count() - 1)
    p.starmap(_do_upload, args)


# -------------------------------------------------
# TENSORFLOW UPLOAD
# -------------------------------------------------

@task
def tf_state(ctx, host=None, knative=True):
    """
    Upload TF experiment state
    """
    data_dir = join(FUNC_DIR, "tf", "data")
    model_file = "mobilenet_v1_1.0_224.tflite"
    host, _ = get_upload_host_port(host, None)

    _do_upload(data_dir, model_file, "tf", host, key="mobilenet_v1")


@task
def tf_upload(ctx, host=None, local_copy=False):
    """
    Upload TF experiment data
    """
    host, port = get_upload_host_port(host, None)

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
