import requests


def do_post(url, input, headers=None, quiet=False, json=False):
    # NOTE: Using python to do this is slow compared with running curl
    # directly on the command line (or some other purpose-built tool).
    # As a result this mustn't be used for performance testing
    print("POST URL    : {}".format(url))
    print("POST Headers: {}".format(headers))
    print("POST JSON   : {}".format(json))
    print("POST Data   : {}".format(input))

    if json:
        response = requests.post(url, json=input, headers=headers)
    else:
        response = requests.post(url, data=input, headers=headers)

    if response.status_code >= 400:
        print("Request failed: status = {}".format(response.status_code))
    elif response.text and not quiet:
        print(response.text)
    elif not quiet:
        print("Empty response")

    return response.text

