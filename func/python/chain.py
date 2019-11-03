from pyfaasm.core import getFunctionIdx, getInput, chainThisWithInput, registerFunction, PYTHON_LOCAL_CHAINING, awaitCall

def register_funcs():
    pass

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
    idx = getFunctionIdx()
    print("Got function index {}".format(idx))

    if PYTHON_LOCAL_CHAINING:
        print("Running native python")
        registerFunction(1, chainOne)
        registerFunction(2, chainTwo)
    else:
        print("Not running native python")

    if idx == 0:
        print("Main chaining entry point")
        call_a = chainThisWithInput(1, b'1234')
        call_b = chainThisWithInput(2, b'5678')

        print("Awaiting calls {} and {}".format(call_a, call_b))

        res_a = awaitCall(call_a)
        res_b = awaitCall(call_b)

        if res_a != 0 or res_b != 0:
            print("Chained functions failed: {} {}".format(res_a, res_b))
            exit(1)

    elif idx == 1:
        chainOne(getInput())

    elif idx == 2:
        chainTwo(getInput())


if __name__ == "__main__":
    main_func()
