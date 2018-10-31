from os import mkdir
from os.path import exists
from os.path import join
from subprocess import call

from requests import get

from tasks.env import PROJ_ROOT

DOWNLOAD_DIR = join(PROJ_ROOT, "download")


def download_proj(url, filename, extension="tar.gz", tar_args="-xvf", extract_file=None):
    extract_dir = join(DOWNLOAD_DIR, extract_file) if extract_file else join(DOWNLOAD_DIR, filename)
    tar_filename = "{}.{}".format(filename, extension)
    tar_file = join(DOWNLOAD_DIR, tar_filename)

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
    call(cmd, cwd=DOWNLOAD_DIR, shell=True)

    # Create build dir
    if not exists(build_dir):
        mkdir(build_dir)

    return extract_dir, build_dir
