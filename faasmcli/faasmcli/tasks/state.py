from invoke import task

from faasmcli.util.endpoints import get_upload_host_port
from faasmcli.util.state import download_binary_state


@task
def download(ctx, user, key, out_path):
    """
    Downloads a state value to the given file
    """
    host, port = get_upload_host_port(None, None)

    download_binary_state(user, key, out_path, host=host, port=port)
