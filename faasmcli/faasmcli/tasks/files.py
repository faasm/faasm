from invoke import task
import requests

from faasmcli.util.endpoints import get_upload_host_port
from faasmcli.util.upload_util import curl_file


def get_file_url():
    host, port = get_upload_host_port()
    url = "http://{}:{}/file/".format(host, port)
    return url


@task(default=True)
def upload(ctx, in_path, shared_path):
    """
    Uploads a shared file
    """
    url = get_file_url()
    print("Uploading {} to {} (at {})".format(in_path, shared_path, url))

    curl_file(
        url,
        in_path,
        headers={
            "FilePath": shared_path,
        },
    )


@task
def download(ctx, shared_path, out_path):
    """
    Downloads a shared file
    """
    url = get_file_url()
    print("Downloading {} to {} (from {})".format(shared_path, out_path, url))

    resp = requests.get(
        url,
        headers={
            "FilePath": shared_path,
        },
    )

    with open(out_path, "wb") as fh:
        fh.write(resp.content)
