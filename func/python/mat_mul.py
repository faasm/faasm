import pyfaasm.matrix as mat
from pyfaasm.core import get_input, set_output


# Note - slight hack here as PyFaasm matrix multiplication expects top-level
# functions of the names in the matrix module


def divide_and_conquer():
    print("Kicking off divide and conquer")
    mat.divide_and_conquer()
    output = "Matrix multiplication finished"
    set_output(output.encode("utf-8"))


def distributed_divide_and_conquer():
    # Run the worker
    print("Running matrix multiplication worker")
    input_bytes = get_input()
    mat.distributed_divide_and_conquer(input_bytes)


def faasm_main():
    divide_and_conquer()

    return 0
