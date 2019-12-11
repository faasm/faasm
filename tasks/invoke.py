from invoke import task

from tasks.util.invoke import invoke_impl, status_call_impl, flush_call_impl
from tasks.util.endpoints import get_kubernetes_host_port


@task
def invoke(ctx, user, func,
           host=None,
           port=None,
           input=None,
           py=False,
           async=False,
           knative=True,
           native=False,
           ibm=False,
           poll=False
           ):
    invoke_impl(user, func, host=host, port=port, input=input, py=py, async=async,
                knative=knative, native=native, ibm=ibm, poll=poll)


@task
def status(ctx, call_id, host=None, port=None):
    k8s_host, k8s_port = get_kubernetes_host_port()
    host = host if host else k8s_host
    port = port if port else k8s_port

    status_call_impl(call_id, host, port)


@task
def flush(ctx):
    host, port = get_kubernetes_host_port()
    host = host if host else "127.0.0.1"
    port = port if port else 8080

    flush_call_impl(host, port)
