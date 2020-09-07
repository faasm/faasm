import subprocess
from os import makedirs
from os.path import exists
from os.path import join
from subprocess import call

from requests import get

from faasmcli.util.env import FAASM_HOME


def download_proj(url, filename, extension="tar.gz", tar_args="-xf", extract_file=None):
    if not exists(FAASM_HOME):
        makedirs(FAASM_HOME)

    extract_dir = (
        join(FAASM_HOME, extract_file) if extract_file else join(FAASM_HOME, filename)
    )
    tar_filename = "{}.{}".format(filename, extension)
    tar_file = join(FAASM_HOME, tar_filename)

    build_dir = join(extract_dir, "build")

    # Ignore if already exists
    if exists(extract_dir):
        return extract_dir, build_dir

    # Download the file
    with open(tar_file, "wb") as fh:
        response = get(url)
        fh.write(response.content)

    # Extract
    cmd = "tar {} {}".format(tar_args, tar_filename)
    call(cmd, cwd=FAASM_HOME, shell=True)

    # Create build dir
    if not exists(build_dir):
        makedirs(build_dir)

    return extract_dir, build_dir


def get_file(url, output_path):
    cmd = ["curl", "-X", "GET", url, "-o", output_path]

    cmd = " ".join(cmd)
    print(cmd)
    res = subprocess.call(cmd, shell=True)

    if res == 0:
        print("Successful GET file from {} to {}".format(url, output_path))
    else:
        raise RuntimeError("Failed GETting file from {} to {}".format(url, output_path))
