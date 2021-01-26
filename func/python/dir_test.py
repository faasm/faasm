import os
from pyfaasm.core import read_input, write_output, get_input_len


def faasm_main():
    input_len = get_input_len()
    i = read_input(input_len)

    if not i:
        print("Input must be a path")
        return 1

    path = i.decode("utf-8")
    print("Path = {}, cwd = {}".format(path, os.getcwd()))

    dirlist = os.listdir(path)
    dirlist.sort()
    dir_list_str = ",".join(dirlist)

    print(dir_list_str)

    write_output(bytes(dir_list_str, "utf-8"))

    return 0
