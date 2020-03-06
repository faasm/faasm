from os.path import join

import numpy as np
from invoke import task
from numpy import int32
from pyfaasm.config import MatrixConf
from pyfaasm.matrix import random_matrix
from pyfaasm.matrix_data import subdivide_matrix_into_files

from tasks.util.matrices import get_matrix_dir, MATRIX_CONF_STATE_KEY, SUBMATRICES_KEY_A, SUBMATRICES_KEY_B


@task
def generate_all(ctx):
    splits = [2]
    sizes = [100, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000]
    for n_splits in splits:
        for matrix_size in sizes:
            generate(ctx, matrix_size, n_splits)


@task
def generate(ctx, matrix_size, n_splits):
    matrix_size = int(matrix_size)
    n_splits = int(n_splits)
    data_dir = get_matrix_dir(matrix_size, n_splits)

    conf = MatrixConf(matrix_size, n_splits)

    params_path = join(data_dir, MATRIX_CONF_STATE_KEY)

    # Write params to file
    print("Generating {}x{} matrix with {} splits".format(matrix_size, matrix_size, n_splits))
    params = np.array((matrix_size, n_splits), dtype=int32)
    with open(params_path, "wb") as fh:
        fh.write(params.tobytes())

    mat_a = random_matrix(matrix_size)
    mat_b = random_matrix(matrix_size)
    subdivide_matrix_into_files(conf, mat_a, data_dir, SUBMATRICES_KEY_A)
    subdivide_matrix_into_files(conf, mat_b, data_dir, SUBMATRICES_KEY_B)
