import multiprocessing
from json import dumps

import requests
from invoke import task

# NOTE: Using python to do this is slow compared with running curl
# directly on the command line (or some other purpose-built tool).
# As a result this mustn't be used for performance testing
from tasks.util.config import get_faasm_config


def _do_post(url, input, headers=None):
    response = requests.post(url, data=input, headers=headers)

    if response.status_code >= 400:
        print("Request failed: status = {}".format(response.status_code))
    elif response.text:
        print(response.text)
    else:
        print("Empty response")


def _do_invoke(user, func, host, port, func_type, input=None):
    url = "http://{}:{}/{}/{}/{}".format(host, port, func_type, user, func)
    print("Invoking {}".format(url))
    _do_post(url, input)


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
           ibm=False,
           legacy=False,
           status=False,
           callid=None
           ):

    faasm_config = get_faasm_config()

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

    port = port if port else (8080 if knative or ibm else 8001)

    if ibm:
        host = faasm_config["IBM"]["k8s_subdomain"]

    url = "http://{}:{}/".format(host, port)

    msg = {
        "user": user,
        "function": func,
        "async": async,
    }

    if status:
        assert callid is not None, "If requesting status must provide call ID too"
        msg["status"] = True
        msg["id"] = int(callid)

    if py:
        msg["python"] = True

    if input:
        msg["input_data"] = input

    # For knative must specify which function in the header
    headers = {
        "Host": "faasm-worker.faasm.example.com"
    }

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

    # IBM must call init
    if ibm:
        _do_post("http://{}:{}/init/".format(host, port), msg_json)

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
                raise RuntimeError("Must specify knative or legacy")

            p.starmap(_do_invoke, args_list)
        else:

            if ibm or knative:
                _do_post(url, msg_json, headers=headers)
            elif legacy:
                _do_invoke(user, func, host, port, prefix, input=input)
            else:
                raise RuntimeError("Must specify knative or legacy")
