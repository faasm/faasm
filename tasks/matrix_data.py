import numpy as np
from numpy import int32
from pyfaasm.matrix import load_matrix_conf_from_state, subdivide_matrix_into_file, write_matrix_params_to_state

from tasks.util.matrices import get_params_file, get_mat_a_file, get_mat_b_file, get_result_file


def generate_all_matrix_data():
    splits = [3, 4]
    sizes = [1000, 2000, 3000, 4000, 5000, 6000]
    for n_splits in splits:
        for matrix_size in sizes:
            generate_matrix_data(matrix_size, n_splits)


def generate_matrix_data(matrix_size, n_splits):
    matrix_size = int(matrix_size)
    n_splits = int(n_splits)

    # Set up parameters locally (needed to run pyfaasm)
    write_matrix_params_to_state(matrix_size, n_splits)
    load_matrix_conf_from_state()

    # Write params to file
    print("Setting up {}x{} matrix with {} splits".format(matrix_size, matrix_size, n_splits))
    params = np.array((matrix_size, n_splits), dtype=int32)
    with open(get_params_file(matrix_size, n_splits), "wb") as fh:
        fh.write(params.tobytes())

    mat_a = np.random.rand(matrix_size, matrix_size)
    mat_b = np.random.rand(matrix_size, matrix_size)
    subdivide_matrix_into_file(mat_a, get_mat_a_file(matrix_size, n_splits))
    subdivide_matrix_into_file(mat_b, get_mat_b_file(matrix_size, n_splits))

    # Zero the result
    zero_result = np.zeros((matrix_size, matrix_size))
    with open(get_result_file(matrix_size, n_splits), "wb") as fh:
        fh.write(zero_result.tobytes())
