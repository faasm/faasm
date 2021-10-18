from invoke import task

from faasmcli.util.call import (
    invoke_impl,
    status_call_impl,
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
    input=None,
    py=False,
    asynch=False,
    knative=True,
    poll=False,
    cmdline=None,
    mpi_world_size=None,
    debug=False,
    sgx=False,
):
    """
    Invoke a function
    """
    res = invoke_impl(
        user,
        func,
        input=input,
        py=py,
        asynch=asynch,
        knative=knative,
        poll=poll,
        cmdline=cmdline,
        mpi_world_size=mpi_world_size,
        debug=debug,
        sgx=sgx,
    )

    if asynch:
        print("Call ID: " + str(res))
        with open(LAST_CALL_ID_FILE, "w") as fh:
            fh.write(str(res))


def get_call_id(call_id):
    if not call_id:
        with open(LAST_CALL_ID_FILE) as fh:
            call_id = fh.read()

        if not call_id:
            print("No call ID provided and no last call ID found")
            exit(1)

    return call_id


@task
def status(ctx, call_id=None):
    """
    Get the status of an async function call
    """
    host, port = get_invoke_host_port()
    call_id = get_call_id(call_id)
    status_call_impl(None, None, call_id, host, port, quiet=False)


@task
def exec_graph(ctx, call_id=None, headless=True, output_file=None):
    """
    Get the execution graph for the given call ID
    """
    host, port = get_invoke_host_port()
    call_id = get_call_id(call_id)

    json_str = exec_graph_call_impl(
        None, None, call_id, host, port, quiet=True
    )

    graph = parse_exec_graph_json(json_str)

    if output_file:
        plot_exec_graph(graph, headless=headless, output_file=output_file)
    else:
        plot_exec_graph(graph, headless=headless)
