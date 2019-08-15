from json import dumps

import requests
from invoke import task


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
def knative_invoke(ctx, user, func, host="127.0.0.1", input=None):
    url = "http://{}:8080/".format(host, user, func)

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

    _do_post(url, dumps(msg), headers=headers)


@task
def py_invoke(ctx, user, func, host="127.0.0.1"):
    _do_invoke(user, func, host, 8001, "p")


@task
def invoke_async(ctx, user, func, host="127.0.0.1"):
    _do_invoke(user, func, host, 8001, "fa")


@task
def py_invoke_async(ctx, user, func, host="127.0.0.1"):
    _do_invoke(user, func, host, 8001, "pa")
