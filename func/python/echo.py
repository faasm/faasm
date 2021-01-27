from pyfaasm.core import get_input_len, read_input, write_output


def faasm_main():
    input_len = get_input_len()

    if input_len == 0:
        print("Nothing to echo")
        return 0

    i = read_input(input_len)

    input_str = i.decode("utf-8")

    print("Got input {}".format(input_str))

    write_output(i)

    return 0
