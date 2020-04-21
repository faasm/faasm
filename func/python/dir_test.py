import multiprocessing
import os


def faasm_main():
    print("MP version = {}".format(multiprocessing))

    paths = [
        "/lib/python3.7/multiprocessing/",
        "/lib/python3.7/multiprocessing/__pycache__/"
    ]

    print("CWD = {}".format(os.getcwd()))

    for i, p in enumerate(paths):
        print("Path {}".format(i))
        dirlist = os.listdir(p)
        print("Dirlist fine = {}".format(dirlist))

        for file in dirlist:
            print(file)
