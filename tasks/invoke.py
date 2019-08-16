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
    _do_post(url, input)


@task
def invoke(ctx, user, func, host="127.0.0.1", input=None):
    _do_invoke(user, func, host, 8001, "f", input=input)


@task
def knative_invoke(ctx, user, func, host="127.0.0.1", port=8080, input=None, parallel=False, loops=1):
    url = "http://{}:{}/".format(host, port)

    msg = {
        "user": user,
        "function": func,
    }

    if input:
        msg["input_data"] = input

    # Knative requires specifying which function in the header
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

            args_list = [(url, msg_json, headers) for _ in range(n_workers)]
            p.starmap(_do_post, args_list)
        else:
            _do_post(url, msg_json, headers=headers)


@task
def py_invoke(ctx, user, func, host="127.0.0.1"):
    _do_invoke(user, func, host, 8001, "p")


@task
def invoke_async(ctx, user, func, host="127.0.0.1"):
    _do_invoke(user, func, host, 8001, "fa")


@task
def py_invoke_async(ctx, user, func, host="127.0.0.1"):
    _do_invoke(user, func, host, 8001, "pa")
