import multiprocessing
from json import dumps
from time import sleep

from tasks.util.config import get_faasm_config
from tasks.util.env import PYTHON_USER, PYTHON_FUNC
from tasks.util.http import do_post


def _get_knative_headers(func_name):
    func_name = func_name.replace("_", "-")

    return {
        "Host": "faasm-{}.faasm.example.com".format(func_name)
    }


def _do_invoke(user, func, host, port, func_type, input=None):
    url = "http://{}:{}/{}/{}/{}".format(host, port, func_type, user, func)
    print("Invoking {}".format(url))
    do_post(url, input)


def invoke_impl(user, func,
                host=None,
                port=None,
                input=None,
                parallel=False,
                loops=1,
                py=False,
                async=False,
                knative=True,
                native=False,
                ibm=False,
                poll=False):

    faasm_config = get_faasm_config()

    # Provider-specific stuff
    if ibm:
        host = faasm_config["IBM"]["k8s_subdomain"]
        port = 8080
    elif knative and faasm_config.has_section("Kubernetes"):
        host = faasm_config["Kubernetes"].get("invoke_host", "localhost")
        port = faasm_config["Kubernetes"].get("invoke_port", 8080)
    else:
        host = host if host else "127.0.0.1"
        port = port if port else 8080

        # Polling always requires async
    if poll:
        async = True

        # Create URL and message
    url = "http://{}".format(host)
    if not port == "80":
        url += ":{}".format(port)

    if py:
        msg = {
            "user": PYTHON_USER,
            "function": PYTHON_FUNC,
            "async": async,
            "py_user": user,
            "py_func": func,
            "python": True,
        }
    else:
        msg = {
            "user": user,
            "function": func,
            "async": async,
        }

    if input:
        msg["input_data"] = input

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
        do_post("http://{}:{}/init/".format(host, port), msg)

    if parallel and poll:
        raise RuntimeError("Cannot run poll and parallel")

    # Knative must pass custom headers
    if knative and native:
        headers = _get_knative_headers(func)
    elif knative:
        headers = _get_knative_headers("worker")
    else:
        headers = {}

    for l in range(loops):
        if loops > 1:
            print("LOOP {}".format(l))

        if parallel:
            n_workers = multiprocessing.cpu_count() - 1
            p = multiprocessing.Pool(n_workers)

            if ibm or knative:
                args_list = [(url, msg, headers) for _ in range(n_workers)]
            else:
                raise RuntimeError("Must specify knative, IBM or legacy")

            p.starmap(_do_invoke, args_list)
        elif poll:
            if not knative:
                raise RuntimeError("Poll only supported for knative")

            # Submit initial async call
            async_result = do_post(url, msg, headers=headers, quiet=True)
            try:
                call_id = int(async_result)
            except ValueError:
                print("Could not parse async reponse to int: {}".format(async_result))
                return 1

            print("\n---- Polling {} ----".format(call_id))

            # Poll status until we get success/ failure
            result = ""
            count = 0
            while not result.startswith("SUCCESS") and not result.startswith("FAILED"):
                count += 1
                sleep(2)

                result = status_call_impl(call_id, host, port, quiet=True)
                print("\nPOLL {} - {}".format(count, result))

            print("\n---- Finished {} ----\n".format(call_id))
            print(result)

            if result.startswith("SUCCESS"):
                prefix = "SUCCESS:"
                success = True
            else:
                prefix = "FAILED:"
                success = False

            result = result.replace(prefix, "")
            return success, result

        else:
            if ibm or knative:
                return do_post(url, msg, headers=headers)
            else:
                raise RuntimeError("Must specify knative or legacy")


def status_call_impl(call_id, host, port, quiet=False):
    url = "http://{}".format(host)
    if port != 80:
        url += ":{}/".format(port)

    msg = {
        "status": True,
        "id": call_id,
    }

    # Can always use the faasm worker for getting status
    headers = _get_knative_headers("worker")
    return do_post(url, dumps(msg), headers=headers, quiet=quiet)
