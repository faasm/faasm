import numpy as np

from pyfaasm.core import getFunctionIdx, getInput
from pyfaasm.matrix import divide_and_conquer, distributed_divide_and_conquer, MATRIX_SIZE

# This is the Faasm entry point. If we've been chained,
# we need to invoke the worker, otherwise run the main function
idx = getFunctionIdx()

if idx == 0:
    print("Setting up matrix multiplication problem")
    # Set up the problem
    mat_a = np.random.rand(MATRIX_SIZE, MATRIX_SIZE)
    mat_b = np.random.rand(MATRIX_SIZE, MATRIX_SIZE)

    divide_and_conquer()
else:
    # Run the worker
    print("Running matrix multiplication worker")
    input_bytes = getInput()
    distributed_divide_and_conquer(input_bytes)

