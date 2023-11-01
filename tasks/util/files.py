import glob
from os import makedirs, remove
from os.path import exists
from shutil import rmtree


def clean_dir(dir_path, clean):
    if clean and exists(dir_path):
        rmtree(dir_path)

    if not exists(dir_path):
        makedirs(dir_path)


def glob_remove(glob_pattern, recursive=False, directory=False):
    print("Recursive remove: {}".format(glob_pattern))
    for filename in glob.iglob(glob_pattern, recursive=recursive):
        print("Removing {}".format(filename))
        if directory:
            rmtree(filename)
        else:
            remove(filename)
