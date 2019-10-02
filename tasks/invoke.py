import multiprocessing
from json import dumps

import requests
from invoke import task


# NOTE: Using python to do this is slow compared with running curl
# directly on the command line (or some other purpose-built tool).
# As a result this mustn't be used for performance testing


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
           input=None,
           parallel=False, loops=1,
           py=False,
           ts=False,
           async=False,
           knative=True,
           legacy=False
           ):
    if py:
        prefix = "p"
    elif ts:
        prefix = "t"
    else:
        prefix = "f"

    if async:
        prefix += "a"

    port = 8080 if knative else 8001

    url = "http://{}:{}/".format(host, port)

    msg = {
        "user": user,
        "function": func,
    }

    if py:
        msg["python"] = True

    if input:
        msg["input_data"] = input

    # For knative must specify which function in the header
    headers = {
        "Host": "faasm-worker.faasm.example.com"
    }

    msg_json = dumps(msg)

    for l in range(loops):
        if loops > 1:
            print("LOOP {}".format(l))

        if parallel:
            n_workers = multiprocessing.cpu_count() - 1
            p = multiprocessing.Pool(n_workers)

            if knative:
                args_list = [(url, msg_json, headers) for _ in range(n_workers)]
            elif legacy:
                args_list = [(user, func, host, port, prefix, input) for _ in range(n_workers)]
            else:
                raise RuntimeError("Must specify knative or legacy")

            p.starmap(_do_invoke, args_list)
        else:

            if knative:
                _do_post(url, msg_json, headers=headers)
            elif legacy:
                _do_invoke(user, func, host, port, prefix, input=input)
            else:
                raise RuntimeError("Must specify knative or legacy")