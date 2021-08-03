from invoke import task

from os.path import basename

from faasmcli.util.endpoints import get_upload_host_port
from faasmcli.util.upload_util import curl_file


@task(default=True)
def upload(ctx, in_path, shared_path):
    """
    Upload a shared file to Faasm
    """
    host, port = get_upload_host_port()

    url = "http://{}:{}/file/".format(host, port)

    local_filename = basename(in_path)
    print("Uploading {} to {}".format(local_filename, shared_path))

    curl_file(
        url,
        in_path,
        headers={
            "FilePath": shared_path,
        },
    )
