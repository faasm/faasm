import sys

from pyfaasm.core import setOutput


def main_func():
    output = "Hello! Running Python version {}".format(sys.version)
    print(output)
    setOutput(output.encode("utf-8"))


if __name__ == "__main__":
    main_func()
