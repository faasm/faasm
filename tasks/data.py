from os.path import join

from invoke import task

from tasks.env import HOME_DIR, STATE_S3_BUCKET
from tasks.upload_util import curl_file, upload_file_to_s3


@task
def reuters_upload(ctx, host):
    _do_reuters_upload(host=host)


@task
def reuters_upload_s3(ctx):
    _do_reuters_upload(s3_bucket=STATE_S3_BUCKET)


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
    files = ["vals", "innr", "outr", "size", "nonz"]

    for f in files:
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
