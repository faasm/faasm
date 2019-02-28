from json import dumps
from os.path import join

import boto3
from invoke import task

from tasks.env import HOME_DIR, STATE_S3_BUCKET, AWS_REGION
from tasks.upload_util import curl_file, upload_file_to_s3

_SGD_FUNCS = [
    "sgd_barrier",
    "sgd_begin",
    "sgd_epoch",
    "sgd_finished",
    "sgd_loss",
    "sgd_step",
    "svm_begin",
]

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


@task
def reuters_upload(ctx, host):
    _do_reuters_upload(host=host)


@task
def reuters_upload_s3(ctx):
    _do_reuters_upload(s3_bucket=STATE_S3_BUCKET)


@task
def begin_aws_svm(ctx):
    client = boto3.client("lambda", region_name=AWS_REGION)

    response = client.invoke(
        FunctionName="faasm-dispatch",
        InvocationType="RequestResponse",
        Payload=dumps({
            "user": "sgd",
            "function": "svm_begin",
        }),
    )

    print(response)

    print("\nResponse message: {}".format(response["Payload"].read()))


@task
def clear_aws_queue(ctx):
    client = boto3.client("lambda", region_name=AWS_REGION)

    response = client.invoke(
        FunctionName="faasm-redis",
        InvocationType="RequestResponse",
        Payload=dumps({
            "target": "queue",
        }),
    )

    print(response)

    print("\nResponse message: {}".format(response["Payload"].read()))


@task
def reuters_prepare_aws(ctx):
    client = boto3.client("lambda", region_name=AWS_REGION)

    for state_key in _ALL_REUTERS_STATE_KEYS:
        # Note, hack here where we pass state key as "function"
        payload = {
            "user": "sgd",
            "function": state_key,
        }

        response = client.invoke(
            FunctionName="faasm-state",
            InvocationType="RequestResponse",
            Payload=dumps(payload),
        )

        print(response)


@task
def reuters_codegen(ctx):
    client = boto3.client("lambda", region_name=AWS_REGION)

    for func_name in _SGD_FUNCS:
        payload = {
            "user": "sgd",
            "function": func_name,
        }

        response = client.invoke(
            FunctionName="faasm-codegen",
            InvocationType="RequestResponse",
            Payload=dumps(payload),
        )

        print(response)


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
