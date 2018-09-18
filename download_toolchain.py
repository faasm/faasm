from os import mkdir
from os.path import join, exists
from shutil import rmtree
from subprocess import call

from requests import get

from compile.env import PROJ_ROOT, TOOLCHAIN_DIR

TAR_URL = "https://s3-eu-west-1.amazonaws.com/wasm-toolchain/toolchain.tar.gz"

if __name__ == "__main__":
    tar_file = join(PROJ_ROOT, "toolchain.tar.gz")

    if exists(tar_file):
        print("Tar already exists at {}. Manually delete first.".format(tar_file))
        exit(1)

    # Download the file
    print("Downloading toolchain binaries")
    with open(tar_file, "wb") as fh:
        response = get(TAR_URL)
        fh.write(response.content)

    print("Extracting toolchain binaries")

    # Clear the toolchain dir
    if exists(TOOLCHAIN_DIR):
        rmtree(TOOLCHAIN_DIR)
    mkdir(TOOLCHAIN_DIR)

    # Untar the toolchain to the relevant location
    call(["make", "untar-tools"], cwd=PROJ_ROOT)
