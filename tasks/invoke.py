import requests
from invoke import task


def _do_invoke(user, func, host, func_type):
    url = "http://{}:8001/{}/{}/{}".format(host, func_type, user, func)

    response = requests.post(url)

    if response.status_code >= 400:
        print("Request failed: status = {}".format(response.status_code))
    elif response.text:
        print(response.text)
    else:
        print("Empty response")


@task
def invoke(ctx, user, func, host="127.0.0.1"):
    _do_invoke(user, func, host, "f")


@task
def py_invoke(ctx, user, func, host="127.0.0.1"):
    _do_invoke(user, func, host, "p")


@task
def invoke_async(ctx, user, func, host="127.0.0.1"):
    _do_invoke(user, func, host, "fa")


@task
def py_invoke_async(ctx, user, func, host="127.0.0.1"):
    _do_invoke(user, func, host, "pa")
