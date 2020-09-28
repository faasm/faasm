from invoke import task

from faasmcli.util.call import (
    invoke_impl,
    status_call_impl,
    flush_call_impl,
    exec_graph_call_impl,
)
from faasmcli.util.endpoints import get_invoke_host_port
from faasmcli.util.exec_graph import parse_exec_graph_json, plot_exec_graph

LAST_CALL_ID_FILE = "/tmp/faasm_last_call.txt"


@task(default=True)
def invoke(
    ctx,
    user,
    func,
    host=None,
    port=None,
    input=None,
    py=False,
    asynch=False,
    knative=True,
    poll=False,
    cmdline=None,
    debug=False,
):
    """
    Invoke a function
    """
    res = invoke_impl(
        user,
        func,
        host=host,
        port=port,
        input=input,
        py=py,
        asynch=asynch,
        knative=knative,
        poll=poll,
        cmdline=cmdline,
        debug=debug,
    )

    if asynch:
        print("Call ID: " + str(res))
        with open(LAST_CALL_ID_FILE, "w") as fh:
            fh.write(str(res))


@task
def status(ctx, call_id, host=None, port=None):
    """
    Get the status of an async function call
    """
    k8s_host, k8s_port = get_invoke_host_port()
    host = host if host else k8s_host
    port = port if port else k8s_port

    status_call_impl(None, None, call_id, host, port, quiet=False)


@task
def exec_graph(
    ctx, call_id=None, host=None, port=None, headless=False, output_file=None
):
    """
    Get the execution graph for the given call ID
    """
    k8s_host, k8s_port = get_invoke_host_port()
    host = host if host else k8s_host
    port = port if port else k8s_port

    if not call_id:
        with open(LAST_CALL_ID_FILE) as fh:
            call_id = fh.read()

        if not call_id:
            print("No call ID provided and no last call ID found")
            exit(1)

    json_str = exec_graph_call_impl(
        None, None, call_id, host, port, quiet=True
    )

    graph = parse_exec_graph_json(json_str)
    plot_exec_graph(graph, headless=headless, output_file=output_file)


@task
def flush(ctx, user, function):
    """
    Flush workers
    """
    host, port = get_invoke_host_port()
    host = host if host else "127.0.0.1"
    port = port if port else 8080

    flush_call_impl(host, port, user, function)
