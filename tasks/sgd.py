from os.path import join

from botocore.exceptions import ClientError
from invoke import task

from tasks.upload import upload_func
from tasks.aws import invoke_faasm_lambda, invoke_lambda, deploy_native_lambda_func, deploy_wasm_lambda_func, \
    lambda_concurrency, delete_lambda
from tasks.env import HOME_DIR, STATE_S3_BUCKET
from tasks.upload_util import curl_file, upload_file_to_s3

_MAX_BATCH_TIME = 180
_BATCH_MEMORY = 128
_BATCH_CONCURRENCY = 200

_SGD_FUNCS = {
    "sgd_barrier": {
        "memory": 256,
        "timeout": 30,
        "concurrency": 1,
    },
    "sgd_epoch": {
        "memory": 256,
        "timeout": 30,
        "concurrency": 1,
    },
    "sgd_finished": {
        "memory": 128,
        "timeout": 30,
        "concurrency": 1,
    },
    "sgd_loss": {
        "memory": 128,
        "timeout": 30,
        "concurrency": 1,
    },
    "sgd_step": {
        "memory": _BATCH_MEMORY,
        "timeout": _MAX_BATCH_TIME,
        "concurrency": _BATCH_CONCURRENCY,
    },
    "svm_begin": {
        "memory": 128,
        "timeout": 30,
        "concurrency": 1,
    },
}

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
# SGD
# -------------------------------------------------

@task
def begin_aws_svm(ctx):
    invoke_faasm_lambda(ctx, "sgd", "svm_begin")


@task
def upload_sgd_funcs(ctx, host="localhost", emscripten=False):
    for func_name in _SGD_FUNCS:
        upload_func(ctx, "sgd", func_name, host=host, emscripten=emscripten)


@task
def deploy_sgd_wasm_lambda(ctx):
    for func_name, func_spec in _SGD_FUNCS.items():
        print("Deploying wasm {}".format(func_name))

        deploy_wasm_lambda_func(ctx, "sgd", func_name)


@task
def deploy_sgd_native_lambda(ctx, func=None):
    if func:
        _do_func_deploy(ctx, func, _SGD_FUNCS[func])
    else:
        for func_name, func_spec in _SGD_FUNCS.items():
            _do_func_deploy(ctx, func_name, func_spec)


@task
def zero_sgd_native_lambda(ctx):
    for func_name, func_spec in _SGD_FUNCS.items():
        lambda_concurrency(ctx, "sgd-{}".format(func_name), 0)


def _do_func_deploy(ctx, func_name, func_spec):
    print("Deploying native {}".format(func_name))

    deploy_native_lambda_func(
        ctx,
        "sgd", func_name,
        memory=func_spec["memory"],
        timeout=func_spec["timeout"],
        concurrency=func_spec["concurrency"],
    )


@task
def delete_sgd_native_lambda(ctx, func=None):
    if func:
        _do_delete(ctx, func)
    else:
        for func_name, func_spec in _SGD_FUNCS.items():
            _do_delete(ctx, func_name)


def _do_delete(ctx, func_name):
    lambda_name = "sgd-{}".format(func_name)
    try:
        delete_lambda(ctx, lambda_name)
    except ClientError:
        print("Failed to delete {}. Continuing".format(lambda_name))


# -------------------------------------------------
# REUTERS DATA
# -------------------------------------------------

@task
def reuters_upload(ctx, host):
    _do_reuters_upload(host=host)


@task
def reuters_upload_s3(ctx):
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
    directory = join(HOME_DIR, "faasm", "data", "reuters")

    # Upload the matrix data
    upload_sparse_matrix(user, "inputs", directory, host=host, s3_bucket=s3_bucket)

    # Upload the categories data
    cat_path = join(directory, "outputs")
    upload_binary_file(user, "outputs", cat_path, host=host, s3_bucket=s3_bucket)

    # Upload the feature counts
    counts_path = join(directory, "feature_counts")
    upload_binary_file(user, "feature_counts", counts_path, host=host, s3_bucket=s3_bucket)


def upload_sparse_matrix(user, key, directory, host=None, s3_bucket=None):
    for f in _SPARSE_MATRIX_PARTS:
        this_key = "{}_{}".format(key, f)
        file_path = join(directory, f)

        upload_binary_file(user, this_key, file_path, host=host, s3_bucket=s3_bucket)


def upload_binary_file(user, key, binary_file, host=None, s3_bucket=None):
    print("Uploading binary file at {} for user {}".format(binary_file, user))

    if s3_bucket:
        s3_key = "{}/{}".format(user, key)
        print("Uploading matrix binary to S3 {} -> {}/{}".format(key, s3_bucket, s3_key))
        upload_file_to_s3(binary_file, s3_bucket, s3_key)
    else:
        url = "http://{}:8002/s/{}/{}".format(host, user, key)
        curl_file(url, binary_file)
