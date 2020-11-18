import sys

from pyfaasm.core import set_output


def faasm_main():
    output = "Hello! Running Python version {}".format(sys.version)
    print(output)
    set_output(output.encode("utf-8"))

    return 0
