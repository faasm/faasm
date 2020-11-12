from pyfaasm.core import chain_this_with_input, await_call


def chain_one(input_bytes):
    expected = b"1234"
    print("Chained 1: {} {}".format(input_bytes, expected))
    if input_bytes != expected:
        exit(1)

    return 0


def chain_two(input_bytes):
    expected = b"5678"
    print("Chained 2: {} {}".format(input_bytes, expected))
    if input_bytes != expected:
        exit(1)

    return 0


def faasm_main():
    print("Main chaining entry point")
    call_a = chain_this_with_input(chain_one, b"1234")
    call_b = chain_this_with_input(chain_two, b"5678")

    print("Awaiting calls {} and {}".format(call_a, call_b))

    res_a = await_call(call_a)
    res_b = await_call(call_b)

    if res_a != 0 or res_b != 0:
        print("Chained functions failed: {} {}".format(res_a, res_b))
        exit(1)

    return 0
