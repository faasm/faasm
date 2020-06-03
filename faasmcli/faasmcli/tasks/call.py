from invoke import task

from faasmcli.util.call import invoke_impl, status_call_impl, flush_call_impl
from faasmcli.util.endpoints import get_invoke_host_port

@task(default=True)
def invoke(ctx, user, func,
           host=None,
           port=None,
           input=None,
           py=False,
           asynch=False,
           knative=True,
           native=False,
           ibm=False,
           poll=False,
           cmdline=None,
           debug=False,
           ):
    """
    Invoke a function
    """
    invoke_impl(user, func, host=host, port=port, input=input, py=py, asynch=asynch,
                knative=knative, native=native, ibm=ibm, poll=poll, cmdline=cmdline, debug=debug)

@task
def status(ctx, call_id, host=None, port=None):
    """
    Get the status of an faasm_async function call
    """
    k8s_host, k8s_port = get_invoke_host_port()
    host = host if host else k8s_host
    port = port if port else k8s_port

    status_call_impl(call_id, host, port)


@task
def flush(ctx):
    """
    Flush workers
    """
    host, port = get_invoke_host_port()
    host = host if host else "127.0.0.1"
    port = port if port else 8080

    flush_call_impl(host, port)
