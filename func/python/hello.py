import sys

from pyfaasm.core import set_output


def main_func():
    output = "Hello! Running Python version {}".format(sys.version)
    print(output)
    set_output(output.encode("utf-8"))
