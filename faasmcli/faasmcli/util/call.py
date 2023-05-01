from faasmcli.util.env import PYTHON_USER, PYTHON_FUNC
from faasmcli.util.http import do_post
from faasmcli.util.endpoints import get_planner_host_port
from faasmcli.util.planner import prepare_planner_msg
from json import loads as json_loads
from json.decoder import JSONDecodeError
from time import sleep

STATUS_SUCCESS = "SUCCESS"
STATUS_FAILED = "FAILED"
STATUS_RUNNING = "RUNNING"

POLL_INTERVAL_MS = 1000


def _do_invoke(user, func, host, port, func_type, input=None):
    url = "http://{}:{}/{}/{}/{}".format(host, port, func_type, user, func)
    print("Invoking {}".format(url))
    do_post(url, input, json=True)


def get_exec_time_from_json(result_json):
    """
    Return the execution time (included in Faasm's response JSON) in seconds
    """
    actual_time = (
        float(int(result_json["finish_ts"]) - int(result_json["start_ts"]))
        / 1000
    )

    return actual_time


def _async_invoke(url, msg, headers=None, host=None, port=None, debug=False):
    # Submit initial async call. This will return a fully-fledged message
    status_code, response = do_post(
        url, msg, headers=headers, quiet=True, json=True
    )
    if status_code >= 400:
        print("Request failed: status = {}".format(status_code))
        print("Request body: {}".format(response))
        return

    try:
        async_result_msg = json_loads(response)
    except JSONDecodeError as e:
        print("Error deserialising JSON response: {}".format(e.msg))
        print("Actual result: {}".format(response))

    call_id = async_result_msg["id"]
    app_id = async_result_msg["appId"]

    if debug:
        print("\n---- Polling {} (app: {}) ----".format(call_id, app_id))

    # Poll status until we get success/ failure
    output = None
    count = 0
    result = STATUS_RUNNING
    while result == STATUS_RUNNING:
        count += 1

        interval = float(POLL_INTERVAL_MS) / 1000
        sleep(interval)

        result, output = status_call_impl(
            async_result_msg, host, port, quiet=True
        )
        if debug:
            print("\nPOLL {} - {}".format(count, result))

    if debug:
        print("\n---- Finished {} (app: {}) ----".format(call_id, app_id))

    try:
        json_output = json_loads(output)
    except JSONDecodeError as e:
        print("Error deserialising JSON output: {}".format(e.msg))
        print("Actual output: {}".format(output))

    if "output_data" in json_output:
        print(json_output["output_data"])
        print(
            "Execution time: {} seconds".format(
                get_exec_time_from_json(json_output)
            )
        )
    else:
        print(json_output)


def invoke_impl(
    user,
    func,
    input=None,
    py=False,
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
        msg["mpi"] = True
        msg["mpi_world_size"] = int(mpi_world_size)

    if graph:
        msg["record_exec_graph"] = graph

    print("Invoking function {}/{} at {}".format(user, func, url))

    planner_msg = prepare_planner_msg("EXECUTE", msg)

    return _async_invoke(url, planner_msg, host=host, port=port, debug=debug)


def status_call_impl(msg, host, port, quiet=False):
    """
    Implements a planner HTTP request to get the state of a running function.
    The most important parameters to identify a message result are the app id
    and the message id, both should be bundled in the msg parameter
    """
    planner_msg = prepare_planner_msg("EXECUTE_STATUS", msg)
    call_status, call_result = _do_single_call(host, port, planner_msg, quiet)

    if call_status == 200 and call_result.startswith("RUNNING"):
        return STATUS_RUNNING, call_result
    elif call_status >= 400:
        return STATUS_FAILED, call_result
    else:
        return STATUS_SUCCESS, call_result


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
