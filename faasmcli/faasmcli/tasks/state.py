from invoke import task

from subprocess import run
from faasmcli.util.upload_util import curl_file
from faasmcli.util.endpoints import get_upload_host_port


@task
def upload(ctx, user, key, in_path):
    """
    Uploads data from file into state
    """
    host, _ = get_upload_host_port()

    print("Uploading state file at {} for user {}".format(in_path, user))

    url = "http://{}:8002/s/{}/{}".format(host, user, key)
    curl_file(url, in_path)


@task
def download(ctx, user, key, out_path):
    """
    Downloads a state value to the given file
    """
    host, port = get_upload_host_port()

    print("Downloading state file {} for user {}".format(key, user))

    url = "http://{}:{}/s/{}/{}".format(host, port, user, key)
    cmd = ["curl", "-X", "GET", url, "-o", out_path]
    cmd = " ".join(cmd)

    print(cmd)
    run(cmd, shell=True, check=True)
