import sys

from pyfaasm.core import write_output


def faasm_main():
    output = "Hello! Running Python version {}".format(sys.version)
    print(output)
    write_output(output.encode("utf-8"))

    return 0
