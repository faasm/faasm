from pyfaasm.core import get_input, set_output


def main_func():
    i = get_input()

    print("Got input {}".format(i))

    set_output(bytes(i))


if __name__ == "__main__":
    main_func()
