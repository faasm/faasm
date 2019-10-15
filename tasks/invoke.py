import multiprocessing
from json import dumps
from time import sleep

import requests
from invoke import task

from tasks.util.config import get_faasm_config


def _get_knative_headers(func_name):
    func_name = func_name.replace("_", "-")

    return {
        "Host": "faasm-{}.faasm.example.com".format(func_name)
    }


def _do_post(url, input, headers=None, quiet=False):
    # NOTE: Using python to do this is slow compared with running curl
    # directly on the command line (or some other purpose-built tool).
    # As a result this mustn't be used for performance testing
    print("POST: {}".format(url))
    response = requests.post(url, data=input, headers=headers)

    if response.status_code >= 400:
        print("Request failed: status = {}".format(response.status_code))
    elif response.text and not quiet:
        print(response.text)
    elif not quiet:
        print("Empty response")

    return response.text


def _do_invoke(user, func, host, port, func_type, input=None):
    url = "http://{}:{}/{}/{}/{}".format(host, port, func_type, user, func)
    print("Invoking {}".format(url))
    _do_post(url, input)


def _do_status_call(call_id, host, port, quiet=False):
    # NOTE - this only works for knative

    url = "http://{}:{}/".format(host, port)

    msg = {
        "status": True,
        "id": call_id,
    }

    # Function name here is "faasm-worker"
    headers = _get_knative_headers("worker")
    return _do_post(url, dumps(msg), headers=headers, quiet=quiet)


@task
def invoke(ctx, user, func,
           host="127.0.0.1",
           port=None,
           input=None,
           parallel=False,
           loops=1,
           py=False,
           ts=False,
           async=False,
           knative=True,
           native=False,
           ibm=False,
           legacy=False,
           poll=False
           ):
    faasm_config = get_faasm_config()

    # Provider-specific stuff
    if ibm:
        host = faasm_config["IBM"]["k8s_subdomain"]
        port = 8080
    elif knative and faasm_config.has_section("Kubernetes"):
        host = faasm_config["Kubernetes"].get("invoke_host", "localhost")
        port = faasm_config["Kubernetes"].get("invoke_port", 8080)
    else:
        host = "localhost"
        port = 8080

    # Polling always requires async
    if poll:
        async = True

    # Legacy requires special URLs
    if legacy:
        if py:
            prefix = "p"
        elif ts:
            prefix = "t"
        else:
            prefix = "f"

        if async:
            prefix += "a"
    else:
        prefix = None

    # Create URL and message
    url = "http://{}:{}/".format(host, port)

    msg = {
        "user": user,
        "function": func,
        "async": async,
    }

    if py:
        msg["python"] = True

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

    msg_json = dumps(msg)

    # IBM must call init first
    if ibm:
        _do_post("http://{}:{}/init/".format(host, port), msg_json)

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
                args_list = [(url, msg_json, headers) for _ in range(n_workers)]
            elif legacy:
                args_list = [(user, func, host, port, prefix, input) for _ in range(n_workers)]
            else:
                raise RuntimeError("Must specify knative, IBM or legacy")

            p.starmap(_do_invoke, args_list)
        elif poll:
            if not knative:
                raise RuntimeError("Poll only supported for knative")

            # Submit initial async call
            async_result = _do_post(url, msg_json, headers=headers, quiet=True)
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

                result = _do_status_call(call_id, host, port, quiet=True)
                print("\nPOLL {} - {}".format(count, result))

            print("\n---- Finished {} ----\n".format(call_id))
            print(result)

        else:
            if ibm or knative:
                _do_post(url, msg_json, headers=headers)
            elif legacy:
                _do_invoke(user, func, host, port, prefix, input=input)
            else:
                raise RuntimeError("Must specify knative or legacy")


@task
def status(ctx, call_id, host="127.0.0.1", port=8080):
    _do_status_call(call_id, host, port)
