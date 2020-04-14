from pyfaasm.core import get_function_idx, get_input, chain_this_with_input, register_function, PYTHON_LOCAL_CHAINING, \
    await_call


def chainOne(input_bytes):
    expected = b'1234'
    print("Chained 1: {} {}".format(input_bytes, expected))
    if input_bytes != expected:
        exit(1)


def chainTwo(input_bytes):
    expected = b'5678'
    print("Chained 2: {} {}".format(input_bytes, expected))
    if input_bytes != expected:
        exit(1)


def main_func():
    idx = get_function_idx()
    print("Got function index {}".format(idx))

    if PYTHON_LOCAL_CHAINING:
        print("Running native python")
        register_function(1, chainOne)
        register_function(2, chainTwo)
    else:
        print("Not running native python")

    if idx == 0:
        print("Main chaining entry point")
        call_a = chain_this_with_input(1, b'1234')
        call_b = chain_this_with_input(2, b'5678')

        print("Awaiting calls {} and {}".format(call_a, call_b))

        res_a = await_call(call_a)
        res_b = await_call(call_b)

        if res_a != 0 or res_b != 0:
            print("Chained functions failed: {} {}".format(res_a, res_b))
            exit(1)

    elif idx == 1:
        chainOne(get_input())

    elif idx == 2:
        chainTwo(get_input())


if __name__ == "__main__":
    main_func()
