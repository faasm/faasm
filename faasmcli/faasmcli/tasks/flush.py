from invoke import task

from faasmcli.util.call import (
    flush_call_impl,
)
from faasmcli.util.endpoints import get_invoke_host_port


@task(default=True)
def function(ctx, user, function):
    """
    Flush functions, state and shared files from all workers
    """
    host, port = get_invoke_host_port()
    flush_call_impl(host, port, user, function)
