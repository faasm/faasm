from pyfaasm.core import get_function_idx, get_input, chain_this_with_input, register_function, \
    PYTHON_LOCAL_CHAINING, await_call


def chain_one(input_bytes):
    expected = b'1234'
    print("Chained 1: {} {}".format(input_bytes, expected))
    if input_bytes != expected:
        exit(1)


def chain_two(input_bytes):
    expected = b'5678'
    print("Chained 2: {} {}".format(input_bytes, expected))
    if input_bytes != expected:
        exit(1)


def faasm_main():
    if PYTHON_LOCAL_CHAINING:
        print("Running native python")
        register_function(1, chain_one)
        register_function(2, chain_two)
    else:
        print("Not running native python")

    print("Main chaining entry point")
    call_a = chain_this_with_input(chain_one, b'1234')
    call_b = chain_this_with_input(chain_two, b'5678')

    print("Awaiting calls {} and {}".format(call_a, call_b))

    res_a = await_call(call_a)
    res_b = await_call(call_b)

    if res_a != 0 or res_b != 0:
        print("Chained functions failed: {} {}".format(res_a, res_b))
        exit(1)
