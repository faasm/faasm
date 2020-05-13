from invoke import task

from faasmcli.util.endpoints import get_upload_host_port
from faasmcli.util.state import download_binary_state, upload_binary_state


@task
def upload(ctx, user, key, in_path):
    """
    Uploads data from file into state
    """
    host, _ = get_upload_host_port(None, None)

    upload_binary_state(user, key, in_path, host=host)


@task
def download(ctx, user, key, out_path):
    """
    Downloads a state value to the given file
    """
    host, port = get_upload_host_port(None, None)

    download_binary_state(user, key, out_path, host=host, port=port)
