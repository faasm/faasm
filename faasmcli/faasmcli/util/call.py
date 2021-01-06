from time import sleep

from faasmcli.util.env import PYTHON_USER, PYTHON_FUNC
from faasmcli.util.http import do_post
from faasmcli.util.endpoints import get_invoke_host_port

STATUS_SUCCESS = "SUCCESS"
STATUS_FAILED = "FAILED"
STATUS_RUNNING = "RUNNING"

POLL_INTERVAL_MS = 1000


def _get_knative_headers(func_name):
    func_name = func_name.replace("_", "-")

    return {"Host": "faasm-{}.faasm.example.com".format(func_name)}


def _do_invoke(user, func, host, port, func_type, input=None):
    url = "http://{}:{}/{}/{}/{}".format(host, port, func_type, user, func)
    print("Invoking {}".format(url))
    do_post(url, input, json=True)


def _async_invoke(url, msg, headers=None, poll=False, host=None, port=None):
    # Submit initial async call
    async_result = do_post(url, msg, headers=headers, quiet=True, json=True)

    try:
        call_id = int(async_result)
    except ValueError:
        raise RuntimeError(
            "Could not parse async response to int: {}".format(async_result)
        )

    # Return the call ID if we're not polling
    if not poll:
        return call_id

    print("\n---- Polling {} ----".format(call_id))

    # Poll status until we get success/ failure
    result = None
    output = None
    count = 0
    while result != STATUS_SUCCESS:
        count += 1

        interval = float(POLL_INTERVAL_MS) / 1000
        sleep(interval)

        result, output = status_call_impl(
            msg["user"], msg["function"], call_id, host, port, quiet=True
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
    knative=True,
    poll=False,
    cmdline=None,
    mpi_world_size=None,
    debug=False,
    sgx=False,
):
    host, port = get_invoke_host_port()

    # Polling always requires asynch
    if poll:
        asynch = True

    # Create URL and message
    url = "http://{}".format(host)
    if not port == "80":
        url += ":{}".format(port)

    if py:
        msg = {
            "user": PYTHON_USER,
            "function": PYTHON_FUNC,
            "async": asynch,
            "py_user": user,
            "py_func": func,
            "python": True,
        }
    else:
        msg = {
            "user": user,
            "function": func,
            "async": asynch,
        }

    if sgx:
        msg["sgx"] = sgx

    if input:
        msg["input_data"] = input

    if cmdline:
        msg["cmdline"] = cmdline

    if mpi_world_size:
        msg["mpi_world_size"] = mpi_world_size

    # Knative must pass custom headers
    if knative:
        headers = _get_knative_headers("worker")
    else:
        headers = {}

    if asynch:
        return _async_invoke(
            url, msg, headers=headers, poll=poll, host=host, port=port
        )
    else:
        return do_post(url, msg, headers=headers, json=True, debug=debug)


def flush_call_impl(host, port):
    msg = {
        "flush": True,
    }
    return _do_single_call(host, port, msg, False)


def status_call_impl(user, func, call_id, host, port, quiet=False):
    msg = {
        "user": user,
        "function": func,
        "status": True,
        "id": int(call_id),
    }
    call_result = _do_single_call(host, port, msg, quiet)

    if call_result.startswith("SUCCESS"):
        return STATUS_SUCCESS, call_result
    elif call_result.startswith("FAILED"):
        return STATUS_FAILED, call_result
    else:
        return STATUS_RUNNING, call_result


def exec_graph_call_impl(user, func, call_id, host, port, quiet=False):
    msg = {
        "user": user,
        "function": func,
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

    # If wasm, can always use the faasm worker for getting status
    headers = _get_knative_headers("worker")

    return do_post(url, msg, headers=headers, quiet=quiet, json=True)
