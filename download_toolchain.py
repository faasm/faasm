from os import mkdir
from os.path import join, exists
from shutil import rmtree

from requests import get

from compile.env import PROJ_ROOT

# Note, this URL can be obtained from the waterfall at https://wasm-stat.us/console.
# You need to find a build with green on the first lozenge (i.e. the Linux build), click on
# it then in the pop-up that appears, find "archive binaries" and copy the URL of the "download"
# link
BINARY_URL = "https://storage.googleapis.com/wasm-llvm/builds/linux/35721/wasm-binaries.tbz2"
DEST_DIR = join(PROJ_ROOT, "toolchain")

if __name__ == "__main__":

    # Clear the download dir
    if exists(DEST_DIR):
        rmtree(DEST_DIR)
    mkdir(DEST_DIR)

    # Download the file
    tar_name = join(PROJ_ROOT, "toolchain.tar.gz")
    print("Downloading toolchain binaries")
    with open(tar_name, "wb") as fh:
        response = get(BINARY_URL)
        fh.write(response.content)

    print("Downloaded toolchain {}".format(DEST_DIR))
