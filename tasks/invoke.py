import requests
from invoke import task


@task
def invoke(ctx, user, func, host="127.0.0.1"):
    url = "http://{}:8001/f/{}/{}".format(host, user, func)

    response = requests.post(url)

    if response.status_code >= 400:
        print("Request failed: status = {}".format(response.status_code))
    elif response.text:
        print(response.text)
    else:
        print("Empty response")

