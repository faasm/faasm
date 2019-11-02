from pyfaasm.core import getFunctionIdx, getInput
from pyfaasm.matrix import write_matrix_params_to_state, divide_and_conquer, distributed_divide_and_conquer, \
    SUBMATRICES_KEY_A, SUBMATRICES_KEY_B, subdivide_random_matrix_into_state

# This is the Faasm entry point. If we've been chained,
# we need to invoke the worker, otherwise run the main function
idx = getFunctionIdx()

if idx == 0:
    input_bytes = getInput()
    input_str = input_bytes.decode("utf-8")
    input_parts = [i.strip() for i in input_str.split(" ") if i.strip()]

    matrix_size = int(input_parts[0])
    n_splits = int(input_parts[1])

    # Write params to state
    print("Setting up {}x{} matrix with {} splits".format(matrix_size, matrix_size, n_splits))
    write_matrix_params_to_state(matrix_size, n_splits)

    # Set up the problem
    subdivide_random_matrix_into_state(SUBMATRICES_KEY_A)
    subdivide_random_matrix_into_state(SUBMATRICES_KEY_B)

    divide_and_conquer()
else:
    # Run the worker
    print("Running matrix multiplication worker")
    input_bytes = getInput()
    distributed_divide_and_conquer(input_bytes)

