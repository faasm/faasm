from ctypes import CDLL


def faasm_main():
    this_mod = CDLL(None)
    other_mod = CDLL("/lib/python3.8/lib-dynload/_ctypes.so")

    print("Root module = {}".format(this_mod))
    print("Other module = {}".format(other_mod))

    return 0
