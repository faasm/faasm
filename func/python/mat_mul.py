from pyfaasm.core import getFunctionIdx, getInput, setOutput
from pyfaasm.matrix import divide_and_conquer, distributed_divide_and_conquer


def main_func():
    # This is the Faasm entry point. If we've been chained,
    # we need to invoke the worker, otherwise run the main function
    idx = getFunctionIdx()

    if idx == 0:
        print("Kicking off divide and conquer")
        divide_and_conquer()

        setOutput("Matrix multiplication finished")
    else:
        # Run the worker
        print("Running matrix multiplication worker")
        input_bytes = getInput()
        distributed_divide_and_conquer(input_bytes)


if __name__ == "__main__":
    main_func()
