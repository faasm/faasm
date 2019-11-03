from pyfaasm.core import getFunctionIdx, getInput
from pyfaasm.matrix import divide_and_conquer, distributed_divide_and_conquer, \
    load_matrix_conf_from_state, MatrixConf

# This is the Faasm entry point. If we've been chained,
# we need to invoke the worker, otherwise run the main function
idx = getFunctionIdx()

if idx == 0:
    load_matrix_conf_from_state()
    print("Running matrix multiplication for  up {}x{} matrix with {} splits".format(
        MatrixConf.matrix_size,
        MatrixConf.matrix_size,
        MatrixConf.n_splits
    ))

    divide_and_conquer()
else:
    # Run the worker
    print("Running matrix multiplication worker")
    input_bytes = getInput()
    distributed_divide_and_conquer(input_bytes)
