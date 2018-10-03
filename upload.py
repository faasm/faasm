import argparse
from os.path import join

import requests

from compile.env import PROJ_ROOT

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("user", help="Owner of the function")
    parser.add_argument("function", help="Function name")
    args = parser.parse_args()

    # Put function file in place
    wasm_file = join(PROJ_ROOT, "work", "out.wasm")

    # Upload to Faasm
    url = "http://localhost:8080/f/{}/{}/".format(args.user, args.function)

    print("Uploading file to {}".format(url))
    with open(wasm_file, "rb") as fh:
        # Read in the whole file
        data = fh.read()
        response = requests.put(url, data=data, headers={'Content-Type': 'application/octet-stream'})

    print(response.content)
