from pyfaasm.core import getInput, setOutput


def main_func():
    i = getInput()

    print("Got input {}".format(i))

    setOutput(bytes(i))


if __name__ == "__main__":
    main_func()
