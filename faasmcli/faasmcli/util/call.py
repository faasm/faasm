import pprint

from faasmcli.util.env import PYTHON_USER, PYTHON_FUNC
from faasmcli.util.http import do_post
from faasmcli.util.endpoints import get_planner_host_port
from faasmcli.util.planner import prepare_planner_msg
from json import loads as json_loads
from time import sleep

STATUS_SUCCESS = "SUCCESS"
STATUS_FAILED = "FAILED"
STATUS_RUNNING = "RUNNING"

POLL_INTERVAL_MS = 1000


def _do_invoke(user, func, host, port, func_type, input=None):
    url = "http://{}:{}/{}/{}/{}".format(host, port, func_type, user, func)
    print("Invoking {}".format(url))
    do_post(url, input, json=True)


def _async_invoke(url, msg, headers=None, poll=True, host=None, port=None):
    # Submit initial async call. This will return a fully-fledged message
    async_result = do_post(url, msg, headers=headers, quiet=True, json=True)
    # TODO: put in try-catch
    async_result_msg = json_loads(async_result)

    call_id = async_result_msg["id"]
    app_id = async_result_msg["appId"]

    # Return the call ID if we're not polling
    # TODO: always poll with planner
    if not poll:
        return call_id

    print("\n---- Polling {} (app: {}) ----".format(call_id, app_id))

    # Poll status until we get success/ failure
    result = None
    output = None
    count = 0
    while result != STATUS_SUCCESS:
        count += 1

        interval = float(POLL_INTERVAL_MS) / 1000
        sleep(interval)

        result, output = status_call_impl(
            async_result_msg,
            host,
            port,
            quiet=True
        )
        print("\nPOLL {} - {}".format(count, result))

    print("\n---- Finished {} ----\n".format(call_id))
    print(output)

    if result == STATUS_SUCCESS:
        prefix = "SUCCESS:"
    else:
        prefix = "FAILED:"

    output = output.replace(prefix, "")

    return call_id


def invoke_impl(
    user,
    func,
    input=None,
    py=False,
    asynch=False,
    poll=False,
    cmdline=None,
    mpi_world_size=None,
    debug=False,
    sgx=False,
    graph=False,
):
    host, port = get_planner_host_port()

    # Create URL and message
    url = "http://{}".format(host)
    if not port == "80":
        url += ":{}".format(port)

    if py:
        msg = {
            "user": PYTHON_USER,
            "function": PYTHON_FUNC,
            "py_user": user,
            "py_func": func,
            "python": True,
        }
    else:
        msg = {
            "user": user,
            "function": func,
        }

    if sgx:
        msg["sgx"] = sgx

    if input:
        msg["input_data"] = input

    if cmdline:
        msg["cmdline"] = cmdline

    if mpi_world_size:
        msg["mpi_world_size"] = int(mpi_world_size)

    if graph:
        msg["record_exec_graph"] = graph

    print("Invoking function at {}".format(url))

    print("Payload:")
    pprint.pprint(msg)

    planner_msg = prepare_planner_msg("EXECUTE", msg)

    print("Actual message:")
    pprint.pprint(planner_msg)

    return _async_invoke(url, planner_msg, host=host, port=port)


def status_call_impl(msg, host, port, quiet=False):
    """
    Implements a planner HTTP request to get the state of a running function.
    The most important parameters to identify a message result are the app id
    and the message id, both should be bundled in the msg parameter
    """
    planner_msg = prepare_planner_msg("EXECUTE_STATUS", msg)
    call_result = _do_single_call(host, port, planner_msg, quiet)

    if call_result.startswith("SUCCESS"):
        return STATUS_SUCCESS, call_result
    elif call_result.startswith("FAILED"):
        return STATUS_FAILED, call_result
    else:
        return STATUS_RUNNING, call_result


def exec_graph_call_impl(call_id, host, port, quiet=False):
    msg = {
        "user": "",
        "function": "",
        "exec_graph": True,
        "id": int(call_id),
    }
    call_result = _do_single_call(host, port, msg, quiet)

    if not quiet:
        print(call_result)

    return call_result


def _do_single_call(host, port, msg, quiet):
    url = "http://{}".format(host)
    if port != 80:
        url += ":{}/".format(port)

    return do_post(url, msg, quiet=quiet, json=True)
