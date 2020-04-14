from pyfaasm.core import get_function_idx, get_input, set_output
from pyfaasm.matrix import divide_and_conquer, distributed_divide_and_conquer


def main_func():
    # This is the Faasm entry point. If we've been chained,
    # we need to invoke the worker, otherwise run the main function
    idx = get_function_idx()

    if idx == 0:
        print("Kicking off divide and conquer")
        divide_and_conquer()

        output = "Matrix multiplication finished"
        set_output(output.encode("utf-8"))
    else:
        # Run the worker
        print("Running matrix multiplication worker")
        input_bytes = get_input()
        distributed_divide_and_conquer(input_bytes)


if __name__ == "__main__":
    main_func()
