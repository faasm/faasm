import subprocess
from os.path import join
from subprocess import call

import boto3

from faasmcli.util.env import AWS_REGION

_s3 = None


def _get_s3():
    global _s3
    if _s3 is None:
        _s3 = boto3.resource("s3", region_name=AWS_REGION)

    return _s3


def upload_file_to_s3(file_path, s3_bucket, s3_key, public=False):
    s3 = _get_s3()
    kwargs = {"ExtraArgs": {"ACL": "public-read"}} if public else {}
    s3.Bucket(s3_bucket).upload_file(file_path, s3_key, **kwargs)


def upload_file_to_ibm(file_path, bucket_name, key):
    cmd = [
        "ibmcloud", "cos", "put-object",
        "--bucket", bucket_name,
        "--key", key,
        "--body", file_path,
    ]

    call(" ".join(cmd), shell=True)


def list_files_s3(s3_bucket, prefix):
    s3 = _get_s3()
    b = s3.Bucket(s3_bucket)

    key_strings = []
    for k in b.objects.filter(Prefix=prefix):
        key_strings.append(k.key)

    return key_strings


def download_file_from_s3(s3_bucket, s3_key, file_path, boto=True):
    if boto:
        print("Downloading file using boto - {}".format(file_path))
        s3 = _get_s3()
        s3.Bucket(s3_bucket).download_file(s3_key, file_path)
    else:
        url = "https://s3-{}.amazonaws.com/{}/{}".format(AWS_REGION, s3_bucket, s3_key)
        cmd = "wget -q {} -O {}".format(url, file_path)
        print(cmd)
        subprocess.check_output(cmd, shell=True)


def download_tar_from_s3(s3_bucket, tar_name, tar_dir, boto=True):
    tar_path = join(tar_dir, tar_name)
    download_file_from_s3(s3_bucket, tar_name, tar_path, boto=boto)

    print("Extracting file {} (at {})".format(tar_name, tar_dir))
    subprocess.check_output("tar --no-same-owner -xf {}".format(tar_name), shell=True, cwd=tar_dir)


def download_tar_from_url(url, tar_name, tar_dir):
    tar_path = join(tar_dir, tar_name)

    cmd = "wget -q {} -O {}".format(url, tar_path)
    print(cmd)
    subprocess.check_output(cmd, shell=True)

    print("Extracting file {} (at {})".format(tar_name, tar_dir))
    subprocess.check_output("tar --no-same-owner -xf {}".format(tar_name), shell=True, cwd=tar_dir)


def copy_object_in_s3(s3_bucket, src_key, dest_key, public=False):
    s3 = _get_s3()
    kwargs = {"ACL": "public-read"} if public else {}
    s3.Object(s3_bucket, dest_key).copy_from(
        CopySource="{}/{}".format(s3_bucket, src_key),
        **kwargs
    )


def curl_file(url, file_path, headers=None, quiet=False):
    cmd = [
        "curl",
        "-X", "PUT",
        url,
        "-T", file_path
    ]

    headers = headers if headers else {}
    for key, value in headers.items():
        cmd.append("-H \"{}: {}\"".format(key, value))

    cmd = " ".join(cmd)

    if not quiet:
        print(cmd)

    res = subprocess.call(cmd, shell=True)

    if res == 0 and not quiet:
        print("Successfully PUT file {} to {}".format(file_path, url))
    elif res != 0:
        raise RuntimeError("Failed PUTting file {} to {}".format(file_path, url))
