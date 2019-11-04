from json import dumps

from invoke import task

from tasks.util.config import get_faasm_config
from tasks.util.invoke import invoke_impl, status_call_impl


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
def status(ctx, call_id, host="127.0.0.1", port=8080):
    faasm_config = get_faasm_config()
    host = faasm_config["Kubernetes"].get("invoke_host", "localhost")
    port = faasm_config["Kubernetes"].get("invoke_port", 8080)
    status_call_impl(call_id, host, port)
