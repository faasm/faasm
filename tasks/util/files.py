from os import mkdir
from os.path import exists
from shutil import rmtree


def clean_dir(dir_path, clean):
    if clean and exists(dir_path):
        rmtree(dir_path)

    if not exists(dir_path):
        mkdir(dir_path)
