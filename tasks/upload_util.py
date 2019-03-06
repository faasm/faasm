import subprocess

import boto3

from tasks.env import AWS_REGION

_s3 = None


def upload_file_to_s3(file_path, s3_bucket, s3_key):
    global _s3
    if _s3 is None:
        _s3 = boto3.resource('s3', region_name=AWS_REGION)

    _s3.meta.client.upload_file(file_path, s3_bucket, s3_key)


def curl_file(url, file_path):
    cmd = [
        "curl",
        "-X", "PUT",
        url,
        "-T", file_path
    ]

    cmd = " ".join(cmd)

    res = subprocess.call(cmd, shell=True)

    if res == 0:
        print("Successfully PUT file {} to {}".format(file_path, url))
    else:
        raise RuntimeError("Failed PUTting file {} to {}".format(file_path, url))
