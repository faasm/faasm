from os.path import join, basename

from faasmcli.util.download import get_file
from faasmcli.util.upload_util import upload_file_to_s3, curl_file

_SPARSE_MATRIX_PARTS = [
    "vals", "innr", "outr", "size", "nonz"
]


def upload_sparse_matrix(user, key, directory, host=None, s3_bucket=None):
    assert host or s3_bucket, "Must provide either host or S3 bucket"

    for f in _SPARSE_MATRIX_PARTS:
        this_key = "{}_{}".format(key, f)
        file_path = join(directory, f)

        upload_binary_state(user, this_key, file_path, host=host, s3_bucket=s3_bucket)


def download_binary_state(user, key, output_path, host="localhost", port=8002):
    print("Download binary state file {} for user {}".format(key, user))

    url = "http://{}:{}/s/{}/{}".format(host, port, user, key)
    get_file(url, output_path)


def upload_binary_state(user, key, binary_file, host=None, s3_bucket=None):
    assert host or s3_bucket, "Must provide either host or S3 bucket"

    print("Uploading binary file at {} for user {}".format(binary_file, user))

    if s3_bucket:
        s3_key = "{}/{}".format(user, key)
        print("Uploading matrix binary to S3 {} -> {}/{}".format(key, s3_bucket, s3_key))
        upload_file_to_s3(binary_file, s3_bucket, s3_key)
    else:
        url = "http://{}:8002/s/{}/{}".format(host, user, key)
        curl_file(url, binary_file)


def upload_shared_file(host, local_path, shared_path, quiet=False):
    url = "http://{}:8002/file/".format(host)

    local_filename = basename(local_path)
    print("Uploading {} to {}".format(local_filename, shared_path))

    curl_file(url, local_path, headers={
        "FilePath": shared_path,
    }, quiet=quiet)
