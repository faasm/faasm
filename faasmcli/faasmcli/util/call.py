from time import sleep

from faasmcli.util.config import get_faasm_config
from faasmcli.util.env import PYTHON_USER, PYTHON_FUNC
from faasmcli.util.http import do_post
from faasmcli.util.endpoints import get_invoke_host_port

STATUS_SUCCESS = "SUCCESS"
STATUS_FAILED = "FAILED"
STATUS_RUNNING = "RUNNING"


def _get_knative_headers(func_name):
    func_name = func_name.replace("_", "-")

    return {
        "Host": "faasm-{}.faasm.example.com".format(func_name)
    }


def _do_invoke(user, func, host, port, func_type, input=None):
    url = "http://{}:{}/{}/{}/{}".format(host, port, func_type, user, func)
    print("Invoking {}".format(url))
    do_post(url, input, json=True)


def invoke_impl(user, func,
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
                mpi_world_size=None,
                debug=False,
                poll_interval_ms=1000):
    faasm_config = get_faasm_config()

    # Provider-specific stuff
    if ibm:
        host = faasm_config["IBM"]["k8s_subdomain"]
        port = 8080
    elif knative:
        host, port = get_invoke_host_port()

    # Defaults
    host = host if host else "127.0.0.1"
    port = port if port else 8080
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

    if input:
        msg["input_data"] = input

    if cmdline:
        msg["cmdline"] = cmdline

    if mpi_world_size:
        msg["mpi_world_size"] = mpi_world_size

    # IBM-specific message format
    if ibm:
        faasm_conf = get_faasm_config()
        msg.update({
            "IBM_API_KEY": faasm_conf["IBM"]["api_key"],
            "REDIS_QUEUE_HOST": faasm_conf["IBM"]["redis_host_public"],
            "REDIS_STATE_HOST": faasm_conf["IBM"]["redis_host_public"],
        })

        # Message needs to be nested
        msg = {
            "value": msg,
        }

    # IBM must call init first
    if ibm:
        do_post("http://{}:{}/init/".format(host, port), msg, json=True)

    # Knative must pass custom headers
    if knative and native:
        if py:
            headers = _get_knative_headers("python")
        else:
            headers = _get_knative_headers(func)
    elif knative:
        headers = _get_knative_headers("worker")
    else:
        headers = {}
    if asynch:
        # Submit initial asynch call
        asynch_result = do_post(url, msg, headers=headers, quiet=True, json=True)
        try:
            call_id = int(asynch_result)
        except ValueError:
            raise RuntimeError("Could not parse async response to int: {}".format(asynch_result))

        if not poll:
            # Return the call ID if we're not polling
            return call_id
        else:
            if not knative:
                raise RuntimeError("Poll only supported for knative")

            print("\n---- Polling {} ----".format(call_id))

            # Poll status until we get success/ failure
            result = None
            output = None
            count = 0
            while result != STATUS_SUCCESS:
                count += 1

                interval = float(poll_interval_ms) / 1000
                sleep(interval)

                result, output = status_call_impl(user, func, call_id, host, port, quiet=True, native=native)
                print("\nPOLL {} - {}".format(count, result))

            print("\n---- Finished {} ----\n".format(call_id))
            print(output)

            if result == STATUS_SUCCESS:
                prefix = "SUCCESS:"
                success = True
            else:
                prefix = "FAILED:"
                success = False

            output = output.replace(prefix, "")
            return success, output
    else:
        if ibm or knative:
            return do_post(url, msg, headers=headers, json=True, debug=debug)
        else:
            raise RuntimeError("Must specify knative or ibm")


def flush_call_impl(host, port):
    msg = {
        "flush": True,
    }
    return _do_single_call(None, None, host, port, msg, False, native=False)


def status_call_impl(user, func, call_id, host, port, quiet=False, native=False):
    msg = {
        "status": True,
        "id": int(call_id),
    }
    call_result = _do_single_call(user, func, host, port, msg, quiet, native=native)

    if call_result.startswith("SUCCESS"):
        return STATUS_SUCCESS, call_result
    elif call_result.startswith("FAILED"):
        return STATUS_FAILED, call_result
    else:
        return STATUS_RUNNING, call_result


def exec_graph_call_impl(user, func, call_id, host, port, quiet=False, native=False):
    msg = {
        "exec_graph": True,
        "id": int(call_id),
    }
    call_result = _do_single_call(user, func, host, port, msg, quiet, native=native)

    if not quiet:
        print(call_result)

    return call_result


def _do_single_call(user, func, host, port, msg, quiet, native=False):
    url = "http://{}".format(host)
    if port != 80:
        url += ":{}/".format(port)

    # If wasm, can always use the faasm worker for getting status
    if native:
        headers = _get_knative_headers(func)
    else:
        headers = _get_knative_headers("worker")

    return do_post(url, msg, headers=headers, quiet=quiet, json=True)
