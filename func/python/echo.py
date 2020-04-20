from pyfaasm.core import get_input, set_output


def faasm_main():
    i = get_input()

    print("Got input {}".format(i))

    set_output(bytes(i))
