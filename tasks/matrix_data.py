import numpy as np
from invoke import task
from numpy import int32
from pyfaasm.config import set_up_config
from pyfaasm.matrix_data import subdivide_matrix_into_file

from tasks.util.matrices import get_params_file, get_mat_a_file, get_mat_b_file, get_result_file


@task
def generate_all_matrix_data(ctx):
    splits = [3, 4]
    sizes = [1000, 2000, 3000, 4000, 5000, 6000]
    for n_splits in splits:
        for matrix_size in sizes:
            generate_matrix_data(ctx, matrix_size, n_splits)


@task
def generate_matrix_data(ctx, matrix_size, n_splits):
    matrix_size = int(matrix_size)
    n_splits = int(n_splits)

    # Set up parameters locally (needed to run pyfaasm)
    set_up_config(matrix_size, n_splits)

    # Write params to file
    print("Generating {}x{} matrix with {} splits".format(matrix_size, matrix_size, n_splits))
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
