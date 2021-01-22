import os
from pyfaasm.core import read_input, write_output


def faasm_main():
    path = read_input()

    if not path:
        print("Input must be a path")
        return 1

    path = path.decode()
    print("Path = {}, cwd = {}".format(path, os.getcwd()))

    dirlist = os.listdir(path)
    dirlist.sort()
    dir_list_str = ",".join(dirlist)

    print(dir_list_str)

    write_output(bytes(dir_list_str, "utf-8"))

    return 0
