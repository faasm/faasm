import subprocess

import boto3

from tasks.util.env import AWS_REGION

_s3 = None


def _get_s3():
    global _s3
    if _s3 is None:
        _s3 = boto3.resource('s3', region_name=AWS_REGION)

    return _s3


def upload_file_to_s3(file_path, s3_bucket, s3_key):
    s3 = _get_s3()
    s3.Bucket(s3_bucket).upload_file(file_path, s3_key)


def list_files_s3(s3_bucket, prefix):
    s3 = _get_s3()
    b = s3.Bucket(s3_bucket)

    key_strings = []
    for k in b.objects.filter(Prefix=prefix):
        key_strings.append(k.key)

    return key_strings


def download_file_from_s3(s3_bucket, s3_key, file_path, boto=True):
    if boto:
        s3 = _get_s3()
        s3.Bucket(s3_bucket).download_file(s3_key, file_path)
    else:
        url = "https://s3-{}.amazonaws.com/{}/{}".format(AWS_REGION, s3_bucket, s3_key)
        cmd = "wget -q {} -O {}".format(url, file_path)
        subprocess.check_output(cmd, shell=True)


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
